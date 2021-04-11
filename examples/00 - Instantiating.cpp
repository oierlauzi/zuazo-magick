/*
 * This example shows how instantiate a image source
 * 
 * How to compile:
 * c++ 00\ -\ Instantiating.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lzuazo-compositor -lzuazo-magick -lglfw -ldl -lpthread `Magick++-config --cppflags --cxxflags --ldflags --libs`
 */

#include <zuazo/Instance.h>
#include <zuazo/Player.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Consumers/WindowRenderer.h>
#include <zuazo/Processors/Layers/VideoSurface.h>
#include <zuazo/Sources/Magick.h>

#include <mutex>
#include <iostream>

int main(int argc, const char* argv[]) {
	if(argc != 2) {
		std::cerr << "Usage: " << *argv << " <video_file>" << std::endl;
		std::terminate();
	}

	const std::string path = argv[1];

	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"Magick Example 00",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get() }			//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	//Construct the window object
	Zuazo::Consumers::WindowRenderer window(
		instance, 						//Instance
		"Output Window",				//Layout name
		Zuazo::Math::Vec2i(1280, 720)	//Window size (in screen coordinates)
	);

	//Set the negotiation callback
	window.setVideoModeNegotiationCallback(
		[] (Zuazo::VideoBase&, const std::vector<Zuazo::VideoMode>& compatibility) -> Zuazo::VideoMode {
			auto result = compatibility.front();
			result.setFrameRate(Zuazo::Utils::MustBe<Zuazo::Rate>(result.getFrameRate().highest()));
			return result;
		}
	);

	//Open the window (now becomes visible)
	window.setResizeable(false); //Disable resizeing, as extra care needs to be taken
	window.asyncOpen(lock);

	//Create a layer for rendering to the window
	Zuazo::Processors::Layers::VideoSurface videoSurface(
		instance,
		"Video Surface",
		&window,
		window.getVideoMode().getResolutionValue()
	);

	window.setLayers({videoSurface});
	videoSurface.setScalingMode(Zuazo::ScalingMode::BOXED);
	videoSurface.setScalingFilter(Zuazo::ScalingFilter::CUBIC);
	videoSurface.asyncOpen(lock);

	//Create a video source
	Magick::Image image(path);
	Zuazo::Sources::Magick picture(
		instance, 										//Instance
		path,											//Layout name
		std::move(image)
	);
	picture.setVideoMode(Zuazo::PixelFormats::MAGICK_SOURCE_OPTIMAL_8_A);
	picture.asyncOpen(lock);
	picture.update();

	//Route the signal
	videoSurface << picture;

	//Done!
	lock.unlock();
	getchar();
	lock.lock();

	std::cout << "\nImages's video-mode:\n";
	std::cout << "\t-" << picture.getVideoMode() << "\n";
}