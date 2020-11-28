#include "MagickConversions.h"

namespace Zuazo {

constexpr std::tuple<std::string_view, Magick::StorageType> toMagick(ColorFormat fmt) {
	constexpr std::string_view RGB = "RGB";
	constexpr std::string_view BGR = "BGR";
	constexpr std::string_view GRB = "GRB";
	constexpr std::string_view GBR = "GBR";

	constexpr std::string_view ARGB = "ARGB";
	constexpr std::string_view ABGR = "ABGR";
	constexpr std::string_view AGRB = "AGRB";
	constexpr std::string_view AGBR = "AGBR";
	constexpr std::string_view RGBA = "RGBA";
	constexpr std::string_view BGRA = "BGRA";
	constexpr std::string_view GRBA = "GRBA";
	constexpr std::string_view GBRA = "GBRA";

	switch(fmt) {
	//8bit
	case ColorFormat::R8G8B8:			return { RGB,	Magick::StorageType::CharPixel };
	case ColorFormat::B8G8R8:			return { BGR,	Magick::StorageType::CharPixel };
	case ColorFormat::G8R8B8:			return { GRB,	Magick::StorageType::CharPixel };
	case ColorFormat::G8B8R8:			return { GBR,	Magick::StorageType::CharPixel };

	case ColorFormat::A8R8G8B8:			return { ARGB,	Magick::StorageType::CharPixel };
	case ColorFormat::A8B8G8R8:			return { ABGR,	Magick::StorageType::CharPixel };
	case ColorFormat::A8G8R8B8:			return { AGRB,	Magick::StorageType::CharPixel };
	case ColorFormat::A8G8B8R8:			return { AGBR,	Magick::StorageType::CharPixel };
	case ColorFormat::R8G8B8A8:			return { RGBA,	Magick::StorageType::CharPixel };
	case ColorFormat::B8G8R8A8:			return { BGRA,	Magick::StorageType::CharPixel };
	case ColorFormat::G8R8B8A8:			return { GRBA,	Magick::StorageType::CharPixel };
	case ColorFormat::G8B8R8A8:			return { GBRA,	Magick::StorageType::CharPixel };

	//16bit
	case ColorFormat::R16G16B16:			return { RGB,	Magick::StorageType::ShortPixel };
	case ColorFormat::B16G16R16:			return { BGR,	Magick::StorageType::ShortPixel };
	case ColorFormat::G16R16B16:			return { GRB,	Magick::StorageType::ShortPixel };
	case ColorFormat::G16B16R16:			return { GBR,	Magick::StorageType::ShortPixel };

	case ColorFormat::A16R16G16B16:		return { ARGB,	Magick::StorageType::ShortPixel };
	case ColorFormat::A16B16G16R16:		return { ABGR,	Magick::StorageType::ShortPixel };
	case ColorFormat::A16G16R16B16:		return { AGRB,	Magick::StorageType::ShortPixel };
	case ColorFormat::A16G16B16R16:		return { AGBR,	Magick::StorageType::ShortPixel };
	case ColorFormat::R16G16B16A16:		return { RGBA,	Magick::StorageType::ShortPixel };
	case ColorFormat::B16G16R16A16:		return { BGRA,	Magick::StorageType::ShortPixel };
	case ColorFormat::G16R16B16A16:		return { GRBA,	Magick::StorageType::ShortPixel };
	case ColorFormat::G16B16R16A16:		return { GBRA,	Magick::StorageType::ShortPixel };

	//32bit FP
	case ColorFormat::R32fG32fB32f:		return { RGB,	Magick::StorageType::FloatPixel };
	case ColorFormat::B32fG32fR32f:		return { BGR,	Magick::StorageType::FloatPixel };
	case ColorFormat::G32fR32fB32f:		return { GRB,	Magick::StorageType::FloatPixel };
	case ColorFormat::G32fB32fR32f:		return { GBR,	Magick::StorageType::FloatPixel };

	case ColorFormat::A32fR32fG32fB32f:	return { ARGB,	Magick::StorageType::FloatPixel };
	case ColorFormat::A32fB32fG32fR32f:	return { ABGR,	Magick::StorageType::FloatPixel };
	case ColorFormat::A32fG32fR32fB32f:	return { AGRB,	Magick::StorageType::FloatPixel };
	case ColorFormat::A32fG32fB32fR32f:	return { AGBR,	Magick::StorageType::FloatPixel };
	case ColorFormat::R32fG32fB32fA32f:	return { RGBA,	Magick::StorageType::FloatPixel };
	case ColorFormat::B32fG32fR32fA32f:	return { BGRA,	Magick::StorageType::FloatPixel };
	case ColorFormat::G32fR32fB32fA32f:	return { GRBA,	Magick::StorageType::FloatPixel };
	case ColorFormat::G32fB32fR32fA32f:	return { GBRA,	Magick::StorageType::FloatPixel };

	//64bit FP
	case ColorFormat::R64fG64fB64f:		return { RGB,	Magick::StorageType::DoublePixel };
	case ColorFormat::B64fG64fR64f:		return { BGR,	Magick::StorageType::DoublePixel };
	case ColorFormat::G64fR64fB64f:		return { GRB,	Magick::StorageType::DoublePixel };
	case ColorFormat::G64fB64fR64f:		return { GBR,	Magick::StorageType::DoublePixel };

	case ColorFormat::A64fR64fG64fB64f:	return { ARGB,	Magick::StorageType::DoublePixel };
	case ColorFormat::A64fB64fG64fR64f:	return { ABGR,	Magick::StorageType::DoublePixel };
	case ColorFormat::A64fG64fR64fB64f:	return { AGRB,	Magick::StorageType::DoublePixel };
	case ColorFormat::A64fG64fB64fR64f:	return { AGBR,	Magick::StorageType::DoublePixel };
	case ColorFormat::R64fG64fB64fA64f:	return { RGBA,	Magick::StorageType::DoublePixel };
	case ColorFormat::B64fG64fR64fA64f:	return { BGRA,	Magick::StorageType::DoublePixel };
	case ColorFormat::G64fR64fB64fA64f:	return { GRBA,	Magick::StorageType::DoublePixel };
	case ColorFormat::G64fB64fR64fA64f:	return { GBRA,	Magick::StorageType::DoublePixel };

	default: 							return { "",	Magick::StorageType::UndefinedPixel };
	}
}

inline Chromaticities getChromaticities(const Magick::Image& image) {
	//Get the primaries from the image
	double 	red_x, red_y,
			green_x, green_y,
			blue_x, blue_y,
			white_x, white_y;

	image.chromaRedPrimary(&red_x, &red_y);
	image.chromaGreenPrimary(&green_x, &green_y);
	image.chromaBluePrimary(&blue_x, &blue_y);
	image.chromaWhitePoint(&white_x, &white_y);

	return Chromaticities(
		Math::Vec2f(red_x, red_y),
		Math::Vec2f(green_x, green_y),
		Math::Vec2f(blue_x, blue_y),
		Math::Vec2f(white_x, white_y),
		80.0f //TODO this is sRGB. What if AdobeRGB?
	);
}

}