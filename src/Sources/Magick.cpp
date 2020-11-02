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
		
		Open(const Graphics::Vulkan& vulkan, const Graphics::Frame::Descriptor& conf) 
			: uploader(vulkan, conf)
		{
		}

		~Open() = default;

		void recreate(const Graphics::Frame::Descriptor& conf) {
			uploader = Graphics::Uploader(uploader.getVulkan(), conf);
		}

		Zuazo::Video flush(::Magick::Image& image) const {
			const auto result = uploader.acquireFrame();
			assert(result);

			const auto& pixelData = result->getPixelData();
			const auto& resolution = result->getDescriptor().resolution;

			//Decide the format
			const auto [map, storageType] = toMagick(result->getDescriptor().colorFormat);

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
		assert(!opened);
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);

		//Open it
		opened = Utils::makeUnique<Open>(
			magick.getInstance().getVulkan(),
			magick.getVideoMode() ? magick.getVideoMode().getFrameDescriptor() : Graphics::Frame::Descriptor()
		);

		//Upload the current image
		update();
	}

	void close(ZuazoBase& base) {
		assert(opened);
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);

		videoOut.reset();
		opened.reset();
	}

	void update() {
		auto& magick = owner.get();

		//Set videomode compatibility
		magick.setVideoModeCompatibility(createVideoModeCompatibility(
			magick.getInstance(),
			image
		));

		//Refresh the output if needed
		if(opened) {
			if(magick.getVideoMode()) {
				videoOut.push(opened->flush(image));
			} else {
				videoOut.reset();
			}
		}
	}

	void videoModeCallback(VideoBase& base, const VideoMode& videoMode) {
		auto& magick = static_cast<Magick&>(base);
		assert(&owner.get() == &magick);

		//Update uploader
		if(opened) {
			opened->recreate(
				videoMode ? videoMode.getFrameDescriptor() : Graphics::Frame::Descriptor()
			);
		}
	}



	::Magick::Image& getImage() {
		return image;
	}

	const ::Magick::Image& getImage() const {
		return image;
	}

private:
	static std::vector<VideoMode> createVideoModeCompatibility(	const Instance& instance, 
																const ::Magick::Image& image )
	{
		std::vector<VideoMode> result;

		const Utils::MustBe<Rate> frameRate(Rate(0, 1)); //Static, 0 Hz
		const Utils::MustBe<Resolution> resolution(Resolution(image.columns(), image.rows()));
		const Utils::MustBe<AspectRatio> pixelAspectRatio(AspectRatio(image.density().height(), image.density().width())); //Note that height comes first as a higher density means a lower pixel size
		const Utils::MustBe<ColorPrimaries> colorPrimaries(ColorPrimaries::BT709); //TODO
		const Utils::MustBe<ColorModel> colorModel(ColorModel::RGB); //Always RGB
		const Utils::MustBe<ColorTransferFunction> colorTransferFunction(ColorTransferFunction::IEC61966_2_1); //sRGB
		const Utils::MustBe<ColorSubsampling> colorSubsampling(ColorSubsampling::RB_444); //No subsampling
		const Utils::MustBe<ColorRange> colorRange(ColorRange::FULL); //No need for headroom and footroom
		Utils::Discrete<ColorFormat> colorFormats;

		const auto uploaderFormatSupport = Graphics::Uploader::getSupportedFormats(instance.getVulkan());
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
						VideoMode videoMode,
						::Magick::Image image )
	: Utils::Pimpl<MagickImpl>({}, *this, std::move(image))
	, ZuazoBase(instance, 
				std::move(name),
				{},
				std::bind(&MagickImpl::moved, std::ref(**this), std::placeholders::_1),
				std::bind(&MagickImpl::open, std::ref(**this), std::placeholders::_1),
				std::bind(&MagickImpl::close, std::ref(**this), std::placeholders::_1),
				std::bind(&MagickImpl::update, std::ref(**this)) )
	, VideoBase(
		std::move(videoMode),
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