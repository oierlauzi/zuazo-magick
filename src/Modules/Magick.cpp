#include <zuazo/Modules/Magick.h>

#include <cassert>

namespace Zuazo::Modules {

std::unique_ptr<Magick> Magick::s_singleton;

Magick::Magick() 
	: Instance::Module(std::string(name), version)
{
}

Magick::~Magick() = default;


const Magick& Magick::get() {
	if(!s_singleton) {
		s_singleton = std::unique_ptr<Magick>(new Magick);
	}

	assert(s_singleton);
	return *s_singleton;
}

}