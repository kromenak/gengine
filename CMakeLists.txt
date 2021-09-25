cmake_minimum_required(VERSION 3.17)

project(gengine VERSION 0.0.2
                DESCRIPTION "Gabriel Knight 3 game engine"
                LANGUAGES C CXX)

# Build for C++14, standard required, no extensions.
# Using global "set" rather thant target-specific commands b/c I want all my targets to use the same rules for these.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler warning settings (TODO: a lot of stuff to clean up after adding this!)
#if(MSVC)
#  add_compile_options(/W3 /wd4065)
#else()
#  add_compile_options(-Wall -Wextra -pedantic)
#endif()

# Get all cpp/h files in the Source directory using GLOB.
# TODO: Using file(GLOB) is not recommended for generating source file list - you are supposed to explicitly list!
# TODO: Some people use Python scripts or just manually do this...gotta decide on a solution. 
file(GLOB_RECURSE source_files CONFIGURE_DEPENDS 
    "Source/*.cpp" "Source/*.h" "Source/*.cc" "Source/*.hh"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${source_files})

file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS
    "Assets/*.txt" "Assets/*.shp" "Assets/*.frag" "Assets/*.vert"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${asset_files})

# Add main executable - the game.
add_executable(gk3 ${source_files} ${asset_files})

# Header locations.
target_include_directories(gk3 PRIVATE
    Source
    Source/Assets
    Source/Audio
    Source/Debug
    Source/GK3
    Source/GK3/Actions
    Source/GK3/Actors
    Source/GK3/Animation
    Source/GK3/UI
    Source/Input
    Source/IO
    Source/Math
    Source/ObjectModel
    Source/Platform
    Source/Primitives
    Source/Rendering
    Source/Reports
    Source/Sheep
    Source/Sheep/Compiler
    Source/Sheep/Machine
    Source/UI
    Source/Util
    Source/Util/Threads
    Source/Video
    Source/Video/Decode
    Source/Video/Playback
    Source/Video/Util
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
    # Specify library search directories.
    target_link_directories(gk3 PRIVATE
        Libraries/ffmpeg/lib/win
        Libraries/fmod/lib/win
        Libraries/GLEW/lib/win/x86
        Libraries/SDL/lib/win/x86
        Libraries/zlib/lib/win/x86
    )

    # Link various libraries.
    target_link_libraries(gk3
        avcodec avformat avutil swresample swscale  # ffmpeg
        fmod                                        # fmod
        glew32                                      # GLEW
        zlib                                        # zlib
        SDL2                                        # SD
        opengl32                                    # OpenGL
    )

    # Copies libraries to exe directory. 
    # Gotta use "TO_NATIVE_PATH" because CMake variables use the wrong path separator for Windows.
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} WIN_SOURCE_DIR)
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} WIN_BINARY_DIR)
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND cmd /c ${WIN_SOURCE_DIR}\\Libraries\\CopyLibraries.bat ${WIN_BINARY_DIR} ${WIN_SOURCE_DIR}
        VERBATIM
    )

    # Create a custom "deploy" target that runs the install script.
    # This packages the built game into a zip file in the "Bin" directory, with any copyrighted material excluded.
    # In VS, this target does not appear in the top bar - go to "Solution Explorer -> CMake Targets View" to build this target.
    add_custom_target(deploy)
    add_custom_command(TARGET deploy
        POST_BUILD
        COMMAND cmd /c ${WIN_SOURCE_DIR}\\Scripts\\Install.bat ${WIN_BINARY_DIR} ${WIN_SOURCE_DIR}\\Bin\\$<CONFIG> ${PROJECT_VERSION}
        VERBATIM
    )
    add_dependencies(deploy gk3)
elseif(APPLE)
    # Build a MacOS app (rather than command line tool).
    set_target_properties(gk3 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Xcode/MacOS/Info.plist
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}

        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
        XCODE_ATTRIBUTE_PRODUCT_NAME "Gabriel Knight 3"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.kromenak.gengine"
        XCODE_ATTRIBUTE_MARKETING_VERSION ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION 1
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
        avcodec avformat avutil swresample swscale  # ffmpeg
        fmod                                        # fmod
        GLEW                                        # GLEW
        z                                           # zlib
        ${SDL2}                                     # SDL
        ${COREFOUNDATION_LIB}
        ${OPENGL_LIB}
    )

    # Copies libraries to app bundle post-build.
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/CopyLibraries.sh app
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Libraries
        VERBATIM
    )

    # Create a custom "deploy" target that runs the install script.
    # This packages the built game into a zip file in the "Bin" directory, with any copyrighted material excluded.
    add_custom_target(deploy)
    add_custom_command(TARGET deploy
        POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/Install.sh ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> ${CMAKE_CURRENT_SOURCE_DIR}/Bin/$<CONFIG> ${PROJECT_VERSION}
        VERBATIM
    )
    add_dependencies(deploy gk3)
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

# Add tests subdirectory (creates the "tests" target).
add_subdirectory(Tests)
