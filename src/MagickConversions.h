#pragma once

#include <zuazo/ColorFormat.h>

#include <string_view>
#include <tuple>
#include <Magick++.h>

namespace Zuazo {

constexpr std::tuple<std::string_view, Magick::StorageType> toMagick(ColorFormat fmt);

}

#include "MagickConversions.inl"