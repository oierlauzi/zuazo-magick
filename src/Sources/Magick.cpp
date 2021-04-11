#include <zuazo/Sources/Magick.h>

#include "../MagickConversions.h"

#include <zuazo/Graphics/Uploader.h>

#include <algorithm>

namespace Zuazo::Sources {

/*
 * MagickImpl
 */

struct MagickImpl {
	struct Open {
		Graphics::Uploader					uploader;
		
		Open(const Graphics::Vulkan& vulkan, const Graphics::Frame::Descriptor& frameDesc, const Chromaticities& cromaticities) 
			: uploader(vulkan, frameDesc, cromaticities)
		{
		}

		~Open() = default;

		void recreate(const Graphics::Frame::Descriptor& frameDesc, const Chromaticities& cromaticities) {
			uploader = Graphics::Uploader(uploader.getVulkan(), frameDesc, cromaticities);
		}

		Zuazo::Video flush(::Magick::Image& image) const {
			const auto result = uploader.acquireFrame();
			assert(result);

			const auto& pixelData = result->getPixelData();
			const auto& resolution = result->getDescriptor().getResolution();

			//Decide the format
			const auto [map, storageType] = toMagick(result->getDescriptor().getColorFormat());

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

	::Magick::Image						image;

	Output								videoOut;

	std::unique_ptr<Open>				opened;

	MagickImpl(Magick& magick, ::Magick::Image image)
		: owner(magick)
		, image(std::move(image))
	{
	}

	~MagickImpl() = default;

	void moved(ZuazoBase& base) {
		owner = static_cast<Magick&>(base);
	}

	void open(ZuazoBase& base) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);
		assert(!opened);

		if(magick.getVideoMode()) {
			opened = Utils::makeUnique<Open>(
				magick.getInstance().getVulkan(),
				magick.getVideoMode().getFrameDescriptor(),
				getChromaticities(image)
			);
		}
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);
		assert(!opened);
		assert(lock.owns_lock());

		if(magick.getVideoMode()) {
			lock.unlock();
			auto newOpened = Utils::makeUnique<Open>(
				magick.getInstance().getVulkan(),
				magick.getVideoMode().getFrameDescriptor(),
				getChromaticities(image)
			);
			lock.lock();

			opened = std::move(newOpened);
		}

		assert(lock.owns_lock());
	}

	void close(ZuazoBase& base) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);

		videoOut.reset();
		opened.reset();

		assert(!opened);
	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);
		assert(lock.owns_lock());

		videoOut.reset();
		auto oldOpened = std::move(opened);

		if(oldOpened) {
			lock.unlock();
			oldOpened.reset();
			lock.lock();
		}

		assert(!opened);
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
					videoMode.getFrameDescriptor(),
					getChromaticities(image)					
				);
			} else if(opened && !isValid) {
				//VideoMode has become invalid
				opened.reset();
			} else if(!opened && isValid) {
				//VideoMode has become valid
				opened = Utils::makeUnique<Open>(
					magick.getInstance().getVulkan(),
					videoMode.getFrameDescriptor(),
					getChromaticities(image)
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

		const auto uploaderFormatSupport = Graphics::Uploader::getSupportedFormats(vulkan);
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
				std::bind(&MagickImpl::open, std::ref(**this), std::placeholders::_1),
				std::bind(&MagickImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
				std::bind(&MagickImpl::close, std::ref(**this), std::placeholders::_1),
				std::bind(&MagickImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
				std::bind(&MagickImpl::update, std::ref(**this)) )
	, VideoBase(
		std::bind(&MagickImpl::videoModeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, Signal::SourceLayout<Video>(makeProxy((*this)->videoOut))
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