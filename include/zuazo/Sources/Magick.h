#pragma once

#include <zuazo/Utils/Pimpl.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>
#include <zuazo/Signal/SourceLayout.h>

#include <string_view>

namespace Zuazo::Sources {

struct MagickImpl;
class Magick 
	: public Utils::Pimpl<struct MagickImpl>
	, public ZuazoBase
	, public VideoBase
	, public Signal::SourceLayout<Video>
{
	friend MagickImpl;
public:
	Magick(	Instance& instance, 
			std::string name, 
			VideoMode videoMode = VideoMode::ANY);

	Magick(const Magick& other) = delete;
	Magick(Magick&& other);
	~Magick();

	Magick& 				operator=(const Magick& other) = delete;
	Magick& 				operator=(Magick&& other);

	using ZuazoBase::update;

	void					read(std::string_view path);

};
	
}

namespace Zuazo::PixelFormats {

extern const VideoMode MAGICK_SOURCE_OPTIMAL_8;
extern const VideoMode MAGICK_SOURCE_OPTIMAL_8_A;
extern const VideoMode MAGICK_SOURCE_OPTIMAL_16;
extern const VideoMode MAGICK_SOURCE_OPTIMAL_16_A;
extern const VideoMode MAGICK_SOURCE_OPTIMAL_32f;
extern const VideoMode MAGICK_SOURCE_OPTIMAL_32f_A;
extern const VideoMode MAGICK_SOURCE_OPTIMAL_64f;
extern const VideoMode MAGICK_SOURCE_OPTIMAL_64f_A;

}