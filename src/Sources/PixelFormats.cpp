#include <zuazo/Sources/Magick.h>

namespace Zuazo::PixelFormats {

extern const VideoMode MAGICK_SOURCE_OPTIMAL_8 = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R8G8B8);
extern const VideoMode MAGICK_SOURCE_OPTIMAL_8_A = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R8G8B8A8);
extern const VideoMode MAGICK_SOURCE_OPTIMAL_16 = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R16G16B16);
extern const VideoMode MAGICK_SOURCE_OPTIMAL_16_A = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R16G16B16A16);
extern const VideoMode MAGICK_SOURCE_OPTIMAL_32f = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R32fG32fB32f);
extern const VideoMode MAGICK_SOURCE_OPTIMAL_32f_A = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R32fG32fB32fA32f);
extern const VideoMode MAGICK_SOURCE_OPTIMAL_64f = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R64fG64fB64f);
extern const VideoMode MAGICK_SOURCE_OPTIMAL_64f_A = makeVideoMode(ColorSubsampling::RB_444, ColorRange::FULL, ColorFormat::R64fG64fB64fA64f);

}