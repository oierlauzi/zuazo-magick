#include <zuazo/Sources/Magick.h>

#include "../MagickConversions.h"

#include <zuazo/Graphics/StagedFramePool.h>

#include <algorithm>

namespace Zuazo::Sources {

/*
 * MagickImpl
 */

struct MagickImpl {
	struct Open {
		Graphics::StagedFramePool				framePool;
		
		Open(const Graphics::Vulkan& vulkan, const Graphics::Frame::Descriptor& frameDesc) 
			: framePool(vulkan, frameDesc)
		{
		}

		~Open() = default;

		void recreate(const Graphics::Frame::Descriptor& frameDesc) {
			framePool = Graphics::StagedFramePool(framePool.getVulkan(), frameDesc);
		}

		Zuazo::Video flush(::Magick::Image& image) const {
			const auto result = framePool.acquireFrame();
			assert(result);

			const auto& pixelData = result->getPixelData();
			const auto& resolution = result->getDescriptor()->getResolution();

			//Decide the format
			const auto [map, storageType] = toMagick(result->getDescriptor()->getColorFormat());

			//Copy from the image to the frame
			assert(pixelData.size() == 1);
			assert(map != "");
			assert(storageType != ::Magick::StorageType::UndefinedPixel);
			image.write(
				0, 0,								//Copy area start position
				resolution.width, resolution.height,//Copy area size
				std::string(map), storageType,		//Copy format
				pixelData.front().data() 			//Pixel data
			);

			//Finalize the frame
			result->flush();
			return result;
		}

	};

	using Output = Signal::Output<Video>;

	std::reference_wrapper<Magick> 		owner;

	Output								videoOut;

	::Magick::Image						image;

	std::unique_ptr<Open>				opened;

	MagickImpl(Magick& magick, ::Magick::Image image)
		: owner(magick)
		, videoOut(magick, std::string(Signal::makeOutputName<Video>()))
		, image(std::move(image))
	{
	}

	~MagickImpl() = default;

	void moved(ZuazoBase& base) {
		owner = static_cast<Magick&>(base);
		videoOut.setLayout(base);
	}

	void open(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);
		assert(!opened);

		const auto& videoMode = magick.getVideoMode();
		if(videoMode) {
			//Open it in a unlocked environment
			if(lock) lock->unlock();
			auto newOpened = Utils::makeUnique<Open>(
				magick.getInstance().getVulkan(),
				videoMode.getFrameDescriptor()
			);
			if(lock) lock->lock();

			//Write the changes while locked
			opened = std::move(newOpened);
		}
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		open(base, &lock);
		assert(lock.owns_lock());
	}

	void close(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);

		//Perform changes while locked
		videoOut.reset();
		auto oldOpened = std::move(opened);

		if(oldOpened) {
			//Destroy it in a unlocked environment
			if(lock) lock->unlock();
			oldOpened.reset();
			if(lock) lock->lock();
		}

		assert(!opened);
	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		close(base, &lock);
		assert(lock.owns_lock());
	}

	void update() {
		auto& magick = owner.get();

		//Update the videomode compatibility
		magick.setVideoModeCompatibility(
			createVideoModeCompatibility(magick.getInstance().getVulkan(), image)
		);

		//Refresh the output if needed
		if(opened) {
			assert(magick.getVideoMode());
			videoOut.push(opened->flush(image));
		} else {
			videoOut.reset();
		}
	}

	void videoModeCallback(VideoBase& base, const VideoMode& videoMode) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);

		//Update uploader
		if(magick.isOpen()) {
			const auto isValid = static_cast<bool>(videoMode);

			if(opened && isValid) {
				//VideoMode remains valid
				opened->recreate(
					videoMode.getFrameDescriptor()					
				);
			} else if(opened && !isValid) {
				//VideoMode has become invalid
				opened.reset();
			} else if(!opened && isValid) {
				//VideoMode has become valid
				opened = Utils::makeUnique<Open>(
					magick.getInstance().getVulkan(),
					videoMode.getFrameDescriptor()
				);
			}
		}
	}

	::Magick::Image& getImage() {
		return image;
	}

	const ::Magick::Image& getImage() const {
		return image;
	}

private:
	static std::vector<VideoMode> createVideoModeCompatibility(	const Graphics::Vulkan& vulkan, 
																const ::Magick::Image& image )
	{
		std::vector<VideoMode> result;

		const Utils::MustBe<Rate> frameRate(Rate(0, 1)); //Static, 0 Hz
		const Utils::MustBe<Resolution> resolution(Resolution(image.columns(), image.rows()));
		const Utils::MustBe<AspectRatio> pixelAspectRatio(AspectRatio(image.density().height(), image.density().width())); //Note that height comes first as a higher density means a lower pixel size
		const Utils::MustBe<ColorPrimaries> colorPrimaries(ColorPrimaries::BT709); //This will be ignored if color primaries are defined
		const Utils::MustBe<ColorModel> colorModel(ColorModel::RGB); //Always RGB
		const Utils::MustBe<ColorTransferFunction> colorTransferFunction(ColorTransferFunction::IEC61966_2_1); //sRGB
		const Utils::MustBe<ColorSubsampling> colorSubsampling(ColorSubsampling::RB_444); //No subsampling
		const Utils::MustBe<ColorRange> colorRange(ColorRange::FULL); //No need for headroom and footroom
		Utils::Discrete<ColorFormat> colorFormats;

		const auto uploaderFormatSupport = Graphics::StagedFrame::getSupportedFormats(vulkan);
		for(const auto& format : uploaderFormatSupport) {
			if(toMagick(format) != toMagick(ColorFormat::NONE)) {
				colorFormats.push_back(format);
			}
		}

		result.emplace_back(
			std::move(frameRate),
			std::move(resolution),
			std::move(pixelAspectRatio),
			std::move(colorPrimaries),
			std::move(colorModel),
			std::move(colorTransferFunction),
			std::move(colorSubsampling),
			std::move(colorRange),
			std::move(colorFormats)
		);

		return result;
	}

};



/*
 * Magick
 */

Magick::Magick(	Instance& instance, 
						std::string name, 
						::Magick::Image image )
	: Utils::Pimpl<MagickImpl>({}, *this, std::move(image))
	, ZuazoBase(instance, 
				std::move(name),
				{},
				std::bind(&MagickImpl::moved, std::ref(**this), std::placeholders::_1),
				std::bind(&MagickImpl::open, std::ref(**this), std::placeholders::_1, nullptr),
				std::bind(&MagickImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
				std::bind(&MagickImpl::close, std::ref(**this), std::placeholders::_1, nullptr),
				std::bind(&MagickImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
				std::bind(&MagickImpl::update, std::ref(**this)) )
	, VideoBase(
		std::bind(&MagickImpl::videoModeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, Signal::SourceLayout<Video>((*this)->videoOut.getProxy())
{
}

Magick::Magick(Magick&& other) = default;

Magick::~Magick() = default;

Magick& Magick::operator=(Magick&& other) = default;



::Magick::Image& Magick::getImage() {
	return (*this)->getImage();
}

const ::Magick::Image& Magick::getImage() const {
	return (*this)->getImage();
}
	
}