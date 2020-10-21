cmake_minimum_required(VERSION 3.9)
include(GNUInstallDirs)

#Project info
project(zuazo-magick
	VERSION 0.1.0 
	DESCRIPTION "Bitmap file I/O for Zuazo"
)

#Subdirectories
#add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/doc/doxygen/)

#Check if the needed libraries are installed
find_package(ImageMagick COMPONENTS Magick++ MagickCore REQUIRED)

#Include ImageMagick (Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})

message(STATUS "Zuazo found at: ${ImageMagick_EXECUTABLE_DIR}")

#Set compiler's options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

#Register all source and header files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE INLINE_SOURCES ${PROJECT_SOURCE_DIR}/include/*.inl)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(
	${PROJECT_NAME} PROPERTIES 
		SOVERSION ${PROJECT_VERSION}	
)
#add_dependencies(${PROJECT_NAME} shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${PROJECT_NAME} PRIVATE ${SHADER_INCLUDE_DIR}/)

#Configure ImageMagick 
#Based on: 
#https://stackoverflow.com/questions/38546196/how-can-i-determine-magickcore-quantum-depth-and-magickcore-hdri-enable-during-c
if(ImageMagick_FOUND)
    # Find Imagemagick Library directory
	get_filename_component(MAGICK_LIB_DIR ${ImageMagick_MagickCore_LIBRARY} DIRECTORY)
	
    # Find where Magick++-config lives
	file(GLOB_RECURSE MAGICK_CONFIG FOLLOW_SYMLINKS ${MAGICK_LIB_DIR}/Magick++-config)
	
    # Ask about CXX and lib flags/locations
    set(MAGICK_CONFIG ${MAGICK_CONFIG} CACHE STRING "Path to Magick++-config utility")
    execute_process(COMMAND "${MAGICK_CONFIG}" "--cxxflags" OUTPUT_VARIABLE MAGICK_CXX_FLAGS)

    # Add these to cache
    set(MAGICK_CXX_FLAGS "${MAGICK_CXX_FLAGS}" CACHE STRING "ImageMagick configuration specific compilation flags." )

    # Split into list:
    string(REGEX MATCHALL "([^\ ]+)" MAGICK_CXX_FLAGS "${MAGICK_CXX_FLAGS}")

    # Remove trailing whitespace (CMAKE warns about this)
    string(STRIP "${MAGICK_CXX_FLAGS}" MAGICK_CXX_FLAGS)

    target_compile_options(${PROJECT_NAME} PUBLIC ${MAGICK_CXX_FLAGS})
endif(ImageMagick_FOUND)

# Install library's binary files and headers
install(TARGETS ${PROJECT_NAME} 
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
