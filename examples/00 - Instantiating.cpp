/*
 * This example shows how instantiate an output window
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-magick -lglfw -ldl -lpthread `Magick++-config --cppflags --cxxflags --ldflags --libs`
 */

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Modules/Magick.h>
#include <zuazo/Consumers/Window.h>
#include <zuazo/Sources/Magick.h>

#include <mutex>
#include <iostream>

int main(int argc, char** argv) {
	if(argc != 2) {
		std::cerr << "Invalid usage" << std::endl;
		std::terminate();
	}

	const std::string path = argv[1];

	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"Magick++ Example 00",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get(),
		Zuazo::Modules::Magick::get() }				//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	Magick::Image image(path);
	Zuazo::Sources::Magick picture(
		instance, 										//Instance
		path,											//Layout name
		Zuazo::PixelFormats::MAGICK_SOURCE_OPTIMAL_8_A,	//Video mode limits
		std::move(image)
	);
	picture.open();

	//Construct the desired video mode
	const Zuazo::VideoMode videoMode(
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(25, 1)), //Just specify the desired rate
		Zuazo::Utils::Any<Zuazo::Resolution>(),
		Zuazo::Utils::Any<Zuazo::AspectRatio>(),
		Zuazo::Utils::Any<Zuazo::ColorPrimaries>(),
		Zuazo::Utils::Any<Zuazo::ColorModel>(),
		Zuazo::Utils::Any<Zuazo::ColorTransferFunction>(),
		Zuazo::Utils::Any<Zuazo::ColorSubsampling>(),
		Zuazo::Utils::Any<Zuazo::ColorRange>(),
		Zuazo::Utils::Any<Zuazo::ColorFormat>()	
	);

	//Construct the window object
	Zuazo::Consumers::Window window(
		instance, 						//Instance
		"Output Window",				//Layout name
		videoMode,						//Video mode limits
		Zuazo::Math::Vec2i(1280, 720),	//Window size (in screen coordinates)
		Zuazo::Consumers::Window::NO_MONITOR //No monitor
	);

	//Open the window (now becomes visible)
	window.open();

	window << picture;

	std::cout << "Video-mode support: " << picture.getVideoModeCompatibility().front() << std::endl;

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}