#pragma once

#include <zuazo/ColorFormat.h>
#include <zuazo/ColorPrimaries.h>

#include <string_view>
#include <tuple>
#include <Magick++.h>

namespace Zuazo {

constexpr std::tuple<std::string_view, Magick::StorageType> toMagick(ColorFormat fmt);
Chromaticities getChromaticities(const Magick::Image& image);

}

#include "MagickConversions.inl"