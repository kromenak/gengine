cmake_minimum_required(VERSION 3.17)

project(gengine VERSION 0.0.9
                DESCRIPTION "Gabriel Knight 3 game engine"
                LANGUAGES C CXX)

# Build for C++14, standard required, no extensions.
# Using global "set" so that all targets use the same rules.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For Xcode, generate schemes for targets. This ensures targets appear in target dropdown.
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

# Generate header containing info about build environment.
# Important to put generated file in BINARY_DIR - otherwise, conflicts may occur when multiple projects are generated.
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_STAT_H)
CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)
configure_file("${PROJECT_SOURCE_DIR}/Source/Platform/BuildEnv.h.in" "${CMAKE_CURRENT_BINARY_DIR}/BuildEnv.h")

# Get all cpp/h files in the Source directory using GLOB.
# Supposedly using GLOB is frowned upon, but...it works really well for now sooooo.
file(GLOB_RECURSE source_files CONFIGURE_DEPENDS
    "Source/*.cpp" "Source/*.h" "Source/*.cc" "Source/*.hh"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${source_files})

# Do the same for Assets folder, so they also appear in IDE.
file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS
    "Assets/*.txt" "Assets/*.shp" "Assets/*.frag" "Assets/*.vert"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${asset_files})

# Add main executable - the engine/game.
add_executable(gk3 ${source_files} ${asset_files})

# Add header locations.
# For any folder listed here, you can add headers in that folder without providing a path.
target_include_directories(gk3 PRIVATE
    # Source include dirs
    Source
    Source/Assets
    Source/Audio
    Source/Containers
    Source/Debug
    Source/GK3
    Source/GK3/Actions
    Source/GK3/Actors
    Source/GK3/Animation
    Source/GK3/Mechanisms
    Source/GK3/Scene
    Source/GK3/UI
    Source/GK3/UI/Sidney
    Source/Input
    Source/IO
    Source/Math
    Source/Memory
    Source/ObjectModel
    Source/Platform
    Source/Primitives
    Source/Rendering
    Source/Rendering/Graphics
    Source/Reports
    Source/RTTI
    Source/Sheep
    Source/Sheep/API
    Source/Sheep/Compiler
    Source/Sheep/Machine
    Source/Tools
    Source/UI
    Source/Util
    Source/Util/Threads
    Source/Video
    Source/Video/Decode
    Source/Video/Playback
    Source/Video/Util

    # Library include dirs
    Libraries/ffmpeg/include
    Libraries/Flex/include
    Libraries/fmod/inc
    Libraries/GLEW/include
    Libraries/imgui
    Libraries/imgui/backends
    Libraries/minilzo
    Libraries/stb
    Libraries/zlib/include

    # Required for including BuildEnv.h
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Common defines.
target_compile_definitions(gk3 PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

# Warning/error settings.
if(MSVC)
    target_compile_options(gk3 PRIVATE /Wall
        /wd4061 # Disable "unhandled case in switch with default" warning
        /wd4065 # Disable "switch contains default and no cases" warning
        /wd4100 # Disable "unreferenced formal parameter" warning
        /wd4191 # Disable "unsafe conversion" warning (used by imgui)
        /wd4242
        /wd4263 /wd4264 # Disable "virtual function in base class has same name & different signature" warning
        /wd4355 # Disable "this used in base member init list" warning
        /wd4365
        /wd4505 # Disable "unreferenced function with internal linkage" warning
        /wd4514 # Disable "unused function" warning
        /wd4625 # Disable "copy constructor implicitly deleted" warning
        /wd4626 # Disable "assignment operator implicitly deleted" warning
        /wd4668 # Disable "preprocessor macro not defined" warning
        /wd4710 # Disable "function was not inlined" warning
        /wd4774 # Disable "snprintf format not literal" warning (though this might be good to fix)
        /wd4820 # Disable "padding added" warning
        /wd5026 # Disable "move constructor implicitly deleted" warning
        /wd5027 # Disable "move assignment operator implicitly deleted" warning
        /wd5039 # Disable "pointer to potentially throwing extern c function" warning
        /wd5045 # Disable "Spectre mitigation" warning
        /wd5219 # Disable "implicit conversion from x to y" warning
        /wd5245 # Disable "unreferenced function with internal linkage has been removed" warning
    )
else() # GCC/Clang
    target_compile_options(gk3 PRIVATE
        -Wall -Wextra
        -Wno-comment            # Disable comment formatting warnings.
        -Wno-unused-function    # Disable unused function warnings.
        -Wno-unused-parameter   # Disable unused function parameter warnings.
    )
endif()

# Other compiler flags.
if(MSVC)
    target_compile_definitions(gk3 PRIVATE
        _CRT_SECURE_NO_WARNINGS     # Don't display warnings for using unsecure standard C/C++ functions.
    )
    target_compile_options(gk3 PRIVATE
        /fp:fast    # Optimize floating point math for speed.
        /JMC        # Enable "Just My Code" to skip system/library code when debugging.
        /MP         # Multithreaded compiling - it's faster.
    )
elseif(APPLE)
    #target_compile_options(gk3 PRIVATE
    #    $<$<CONFIG:RelWithDebInfo>:-O>  # Workaround: currently seeing crashes and performance issues with -O2 optimization; use -O1 instead.
    #)
endif()

# Library locations/linking/copying.
if(WIN32)
    target_include_directories(gk3 PRIVATE
        Libraries/SDL/win/include
    )

    # Specify library search directories.
    target_link_directories(gk3 PRIVATE
        Libraries/ffmpeg/lib/win
        Libraries/fmod/lib/win
        Libraries/GLEW/lib/win/x86
        Libraries/SDL/win/lib/x86
        Libraries/zlib/lib/win/x86
    )

    # Link various libraries.
    target_link_libraries(gk3
        avcodec avformat avutil swresample swscale  # ffmpeg
        fmod                                        # fmod
        glew32                                      # GLEW
        zlib                                        # zlib
        SDL2                                        # SDL
        opengl32                                    # OpenGL
    )
elseif(APPLE)
    target_include_directories(gk3 PRIVATE
        Libraries/SDL/SDL2.framework/Headers
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
else() # Linux
    target_include_directories(gk3 PRIVATE
        Libraries/SDL/linux/include
    )

    # Specify library search directories.
    target_link_directories(gk3 PRIVATE
        Libraries/ffmpeg/lib/linux
        Libraries/fmod/lib/linux
        Libraries/GLEW/lib/linux
        Libraries/SDL/linux/lib
        Libraries/zlib/lib/linux
    )

    # Link all the libraries.
    target_link_libraries(gk3
        avcodec avformat avutil swresample swscale  # ffmpeg
        fmod                                        # fmod
        GLEW                                        # GLEW
        z                                           # zlib
        SDL2                                        # SDL
        GL
    )
endif()

# Other project settings
if(WIN32)
    # Use repo root as debugger working directory.
    # This allows running the game without copying Assets/Data to the Build folder.
    set_property(TARGET gk3 PROPERTY
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # Set GK3 as the startup project.
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gk3)
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

    # Set working directory for Run/Profile in Xcode to the root of the repo.
    # This allows running the game without copying Assets/Data folders into the .app.
    set_target_properties(gk3 PROPERTIES
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    # NOTE: CLion doesn't yet have a WORKING_DIRECTORY CMake variable. It must be set manually to the repo root.
endif()

# Post-build commands, to copy any libraries/assets to final locations.
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PLAT_SOURCE_DIR)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} PLAT_BINARY_DIR)
if(WIN32)
    # Convert directories to Windows (with backslash separator).
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND cmd /c ${PLAT_SOURCE_DIR}\\Scripts\\PostBuild.bat ${PLAT_BINARY_DIR}\\$<CONFIG> ${PLAT_SOURCE_DIR}
        VERBATIM
    )
elseif(APPLE)
    # CLion and Xcode differ in where they put the Mac App, and also how they name it. Let's support both!
    set(MAC_APP_CONTENTS "${PLAT_BINARY_DIR}/gk3.app/Contents")
    if(XCODE)
        set(MAC_APP_CONTENTS "${PLAT_BINARY_DIR}/$<CONFIG>/Gabriel Knight 3.app/Contents")
    endif()

    # Copies libraries to app bundle post-build.
    set(LIB_FFMPEG_SRC "${PLAT_SOURCE_DIR}/Libraries/ffmpeg/lib/mac")
    set(MAC_APP_LIBRARIES "${MAC_APP_CONTENTS}/Libraries")
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND mkdir -p "${MAC_APP_LIBRARIES}"
        COMMAND cp "${LIB_FFMPEG_SRC}/libavcodec.dylib" "${MAC_APP_LIBRARIES}"
        COMMAND cp "${LIB_FFMPEG_SRC}/libavformat.dylib" "${MAC_APP_LIBRARIES}"
        COMMAND cp "${LIB_FFMPEG_SRC}/libavutil.dylib" "${MAC_APP_LIBRARIES}"
        COMMAND cp "${LIB_FFMPEG_SRC}/libswresample.dylib" "${MAC_APP_LIBRARIES}"
        COMMAND cp "${LIB_FFMPEG_SRC}/libswscale.dylib" "${MAC_APP_LIBRARIES}"
        COMMAND cp "${PLAT_SOURCE_DIR}/Libraries/fmod/lib/mac/libfmod.dylib" "${MAC_APP_LIBRARIES}"
        COMMAND cp "${PLAT_SOURCE_DIR}/Libraries/GLEW/lib/mac/libGLEW.dylib" "${MAC_APP_LIBRARIES}"
        COMMAND cp "${PLAT_SOURCE_DIR}/Libraries/zlib/lib/mac/libz.dylib" "${MAC_APP_LIBRARIES}"
    )
else() # Linux
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ln -fs ${PLAT_SOURCE_DIR}/Assets ${PLAT_BINARY_DIR}
        COMMAND ln -fs ${PLAT_SOURCE_DIR}/Data ${PLAT_BINARY_DIR}
        #COMMAND ${CMAKE_COMMAND} -E copy ${PLAT_SOURCE_DIR}/Libraries/ffmpeg/lib/linux/libavcodec.so ${PLAT_BINARY_DIR}
        #COMMAND ${CMAKE_COMMAND} -E copy ${PLAT_SOURCE_DIR}/Libraries/ffmpeg/lib/linux/libavformat.so ${PLAT_BINARY_DIR}
        #COMMAND ${CMAKE_COMMAND} -E copy ${PLAT_SOURCE_DIR}/Libraries/ffmpeg/lib/linux/libavutil.so ${PLAT_BINARY_DIR}
        #COMMAND ${CMAKE_COMMAND} -E copy ${PLAT_SOURCE_DIR}/Libraries/ffmpeg/lib/linux/libswresample.so ${PLAT_BINARY_DIR}
        #COMMAND ${CMAKE_COMMAND} -E copy ${PLAT_SOURCE_DIR}/Libraries/ffmpeg/lib/linux/libswscale.so ${PLAT_BINARY_DIR}
        VERBATIM
    )
endif()

# Create deploy target, which creates a final release of the game.
# This packages the built game into a zip file in the "Bin" directory, with any copyrighted material excluded.
add_custom_target(deploy)
if(WIN32)
    # In VS, this target does not appear in the top bar - go to "Solution Explorer -> CMake Targets View" to build this target.
    add_custom_command(TARGET deploy
        POST_BUILD
        COMMAND cmd /c ${PLAT_SOURCE_DIR}\\Scripts\\Deploy.bat ${PLAT_SOURCE_DIR} ${PLAT_BINARY_DIR}\\$<CONFIG> ${PLAT_SOURCE_DIR}\\Bin\\$<CONFIG> ${PROJECT_VERSION}
        VERBATIM
    )
elseif(APPLE)
    add_custom_command(TARGET deploy
        POST_BUILD
        COMMAND ${PLAT_SOURCE_DIR}/Scripts/Deploy.sh ${PLAT_SOURCE_DIR} ${PLAT_BINARY_DIR}/$<CONFIG> ${PLAT_SOURCE_DIR}/Bin/$<CONFIG> ${PROJECT_VERSION} Mac
        VERBATIM
    )
else() # Linux
    add_custom_command(TARGET deploy
        POST_BUILD
        COMMAND ${PLAT_SOURCE_DIR}/Scripts/Deploy.sh ${PLAT_SOURCE_DIR} ${PLAT_BINARY_DIR} ${PLAT_SOURCE_DIR}/Bin/$<CONFIG> ${PROJECT_VERSION} Linux
        VERBATIM
    )
endif()
add_dependencies(deploy gk3)

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

# Add imgui library (source only).
set(IMGUI_SOURCES
    Libraries/imgui/imgui.cpp
    Libraries/imgui/imgui_demo.cpp
    Libraries/imgui/imgui_draw.cpp
    Libraries/imgui/imgui_stdlib.cpp
    Libraries/imgui/imgui_tables.cpp
    Libraries/imgui/imgui_widgets.cpp
    Libraries/imgui/backends/imgui_impl_opengl3.cpp
    Libraries/imgui/backends/imgui_impl_sdl.cpp
)
target_sources(gk3 PRIVATE ${IMGUI_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${IMGUI_SOURCES})

# Add tests subdirectory (creates the "tests" target).
add_subdirectory(Tests)