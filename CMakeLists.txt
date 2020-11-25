cmake_minimum_required(VERSION 3.17)

project(gengine VERSION 0.1
				DESCRIPTION "Gabriel Knight 3 game engine"
				LANGUAGES C CXX)

# Get all cpp/h files in the Source directory using GLOB.
# TODO: Using file(GLOB) is not recommended for generating source file list - you are supposed to explicitly list!
# TODO: Some people use Python scripts or just manually do this...gotta decide on a solution. 
file(GLOB_RECURSE source_files CONFIGURE_DEPENDS 
	"Source/*.cpp" "Source/*.h" "Source/*.cc" "Source/*.hh"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${source_files})

# Add main executable - the game.
add_executable(gk3 ${source_files})

# Require C++14, no extensions, standard required.
target_compile_features(gk3 PUBLIC cxx_std_14)
set_target_properties(gk3 
	PROPERTIES 
		CXX_EXTENSIONS OFF
		CXX_STANDARD_REQUIRED ON
)

# Header locations.
target_include_directories(gk3 
	PRIVATE
		Source
		Source/Audio
		Source/Barn
		Source/Sheep
		Source/Video
		Libraries/ffmpeg/include
		Libraries/Flex/include
		Libraries/fmod/inc
		Libraries/GLEW/include
		Libraries/minilzo
		Libraries/SDL/include
		Libraries/stb
		Libraries/zlib/include
)

# Library locations.
if(WIN32)
	target_link_libraries(gk3 
		#TODO: ffmpeg
		Libraries/fmod/lib/win
		Libraries/GLEW/lib/win/x86
		#TODO: SDL
	)
elseif(APPLE)
	# Build a MacOS app (rather than command line tool).
	set_target_properties(gk3 PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Xcode/MacOS/Info.plist
		XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
		XCODE_ATTRIBUTE_PRODUCT_NAME "Gabriel Knight 3"
		XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.kromenak.gengine"
	)

	# Add xcassets file under "Resources" in project (required for AppIcon to work correctly).
	target_sources(gk3 PRIVATE Xcode/MacOS/Assets.xcassets)
	set_source_files_properties(Xcode/MacOS/Assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

	# Explicitly set rpath to look in a few places relative to the executable.
	# Specifying "build with install rpath" stops CMake from adding individual library paths to rpath (not desired).
	set_target_properties(gk3 PROPERTIES
		INSTALL_RPATH "@executable_path;@executable_path/../Frameworks;@executable_path/../Libraries"
		BUILD_WITH_INSTALL_RPATH TRUE
	)

	# Specify library search directories.
	target_link_directories(gk3 PRIVATE
		Libraries/ffmpeg/lib/mac
		Libraries/fmod/lib/mac
		Libraries/GLEW/lib/mac
		Libraries/zlib/lib/mac
	)

	# Must use find_library for frameworks, such as SDL (rather than target_link_directories).
	# Adding SDL2 as a "source file" under "Frameworks" causes it to be automatically copied to the app bundle on build (exactly what we need).
	find_library(SDL2 SDL2 PATHS Libraries/SDL REQUIRED NO_DEFAULT_PATH)
	target_sources(gk3 PRIVATE ${SDL2})
	set_source_files_properties(${SDL2} PROPERTIES MACOSX_PACKAGE_LOCATION "Frameworks")

	# Find system libraries required by the app.
	find_library(COREFOUNDATION_LIB CoreFoundation)
	find_library(OPENGL_LIB OpenGL)

	# Link all the libraries.
	target_link_libraries(gk3 
		avcodec avformat avutil swresample swscale 	# ffmpeg
		fmod										# fmod
		GLEW										# GLEW
		z 											# zlib
		${SDL2}										# SDL
		${COREFOUNDATION_LIB}
		${OPENGL_LIB}
	)

	# Copies libraries to app bundle post-build.
	add_custom_command(TARGET gk3
		POST_BUILD
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Xcode/CopyLibraries.sh app
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Xcode
	)
endif()

# Add minilzo library (source only).
set(LZO_SOURCES
	Libraries/minilzo/lzoconf.h
	Libraries/minilzo/lzodefs.h
	Libraries/minilzo/minilzo.c
	Libraries/minilzo/minilzo.h
)
target_sources(gk3 PRIVATE ${LZO_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${LZO_SOURCES})

# Add stb library (source only).
set(STB_SOURCES
	Libraries/stb/stb_image_resize.h
	Libraries/stb/stb_image_resize.cpp
)
target_sources(gk3 PRIVATE ${STB_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${STB_SOURCES})

#TODO: Tests via CMake
#file(GLOB test_files CONFIGURE_DEPENDS "Tests/*.hh" "Tests/*.cpp")
#add_executable(tests ${test_files})