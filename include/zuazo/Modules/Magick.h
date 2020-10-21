#pragma once

#include <zuazo/Instance.h>

#include <memory>

namespace Zuazo::Modules {

class Magick final
	: public Instance::Module
{
public:
	~Magick();

	static constexpr std::string_view name = "Magick";
	static constexpr Version version = Version(0, 1, 0);

	static const Magick& 					get();

private:
	Magick();
	Magick(const Magick& other) = delete;

	Magick& 								operator=(const Magick& other) = delete;

	static std::unique_ptr<Magick> 			s_singleton;
};

}