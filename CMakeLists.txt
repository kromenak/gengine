cmake_minimum_required(VERSION 3.17)

# Set Apple architectures (do this before "project" line for correct results).
#TODO: Update this to include arm64 (but need to rebuild dependent libraries).
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "")

# Define G-Engine project.
project(gengine VERSION 0.2.0
                DESCRIPTION "Gabriel Knight 3 game engine"
                LANGUAGES C CXX)

# Build for C++14, standard required, no extensions.
# Using global "set" so that all targets use the same rules.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate platform paths for source and binary directory, used later.
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PLAT_SOURCE_DIR)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} PLAT_BINARY_DIR)

# For Xcode, generate schemes for targets. This ensures targets appear in target dropdown.
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

# CLion and Xcode differ in where they put the Mac App, and also how they name it. Let's support both!
if(APPLE)
    set(MAC_APP_CONTENTS "${PLAT_BINARY_DIR}/gk3.app/Contents")
    if(XCODE)
        set(MAC_APP_CONTENTS "${PLAT_BINARY_DIR}/$<CONFIG>/Gabriel Knight 3.app/Contents")
    endif()
endif()

# Generate header containing info about build environment.
# Important to put generated file in BINARY_DIR - otherwise, conflicts may occur when multiple projects are generated.
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_STAT_H)
CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
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

###################
# COMPILER CONFIG
###################
# Add header locations. For any folder listed here, you can add headers in that folder without providing a path.
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
    Source/GK3/UI/Sidney/Components
    Source/Input
    Source/IO
    Source/Math
    Source/Memory
    Source/ObjectModel
    Source/Persistence
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

    # Required for including BuildEnv.h
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Platform-agnostic defines.
target_compile_definitions(gk3 PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

# Warning/error settings - generally, enable ALL warnings, but disable a subset we don't want.
if(MSVC)
    target_compile_options(gk3 PRIVATE /Wall # Enable ALL warnings...
        # But disable some.
        /wd4005 # macro redefinition
        /wd4061 # unhandled case in switch with default
        /wd4065 # switch contains default and no cases
        /wd4100 # unreferenced formal parameter
        /wd4191 # unsafe conversion (used by imgui)
        /wd4242 # type conversion may result in loss of data
        /wd4263 /wd4264 # virtual function in base class has same name & different signature
        /wd4355 # 'this' used in base member init list
        /wd4365 # signed/unsigned mismatch
        /wd4505 # unreferenced function with internal linkage
        /wd4514 # unused function
        /wd4611 # use of _setjmp
        /wd4625 # copy constructor implicitly deleted
        /wd4626 # assignment operator implicitly deleted
        /wd4668 # preprocessor macro not defined
        /wd4710 # function was not inlined
        /wd4711 # function was automatically inlined
        /wd4774 # snprintf format not literal (this might be good to fix)
        /wd4820 # padding added
        /wd5026 # move constructor implicitly deleted
        /wd5027 # move assignment operator implicitly deleted
        /wd5039 # pointer to potentially throwing extern c function
        /wd5045 # Spectre mitigation
        /wd5219 # implicit conversion from x to y
        /wd5245 # unreferenced function with internal linkage has been removed
    )
else() # GCC/Clang
    target_compile_options(gk3 PRIVATE -Wall -Wextra # Enable ALL warnings...
        # But disable some.
        -Wno-comment            # comment formatting
        -Wno-inconsistent-missing-override # function is not marked override
        -Wno-invalid-offsetof   # offsetof on non-standard-layout type
        -Wno-overloaded-virtual # function hides overloaded virtual function (occurs in flex/bison generated code)
        -Wno-sign-compare       # comparison of integers of different signs
        -Wno-unused-but-set-variable # unused but set variable
        -Wno-unused-function    # unused function
        -Wno-unused-parameter   # unused function parameter
        -Wno-unused-private-field # unused private field
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

#########################
# LIBRARY/LINKING CONFIG
#########################
# Library locations/linking/copying.
set(LIBS_SRC "${PLAT_SOURCE_DIR}/Libraries")
if(WIN32)
    target_include_directories(gk3 PRIVATE
        Libraries/libpng/win/include
        Libraries/SDL/win/include
        Libraries/zlib/win/include
    )

    # Specify library search directories.
    target_link_directories(gk3 PRIVATE
        Libraries/ffmpeg/lib/win
        Libraries/fmod/lib/win
        Libraries/GLEW/lib/win/x86
        Libraries/libpng/win/lib
        Libraries/SDL/win/lib/x86
        Libraries/zlib/win/lib
    )

    # Link various libraries.
    target_link_libraries(gk3
        avcodec avformat avutil swresample swscale  # ffmpeg
        fmod                                        # fmod
        glew32                                      # GLEW
        libpng16                                    # libpng
        zdll                                        # zlib
        SDL2                                        # SDL
        opengl32                                    # OpenGL
    )

    # Copy libraries after build.
    set(LIB_DEST "${PLAT_BINARY_DIR}/$<CONFIG>")
    set(LIB_FFMPEG_SRC "${LIBS_SRC}/ffmpeg/lib/win")
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/avcodec-58.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/avformat-58.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/avutil-56.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/swresample-3.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/swscale-5.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/fmod/lib/win/fmod.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/GLEW/lib/win/x86/glew32.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/libpng/win/lib/libpng16.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/SDL/win/lib/x86/SDL2.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/zlib/win/lib/zlib1.dll" "${LIB_DEST}"
        VERBATIM
    )
elseif(APPLE)
    target_include_directories(gk3 PRIVATE
        Libraries/libpng/mac/include
        Libraries/SDL/SDL2.framework/Headers
        Libraries/zlib/mac/include
    )

    # Specify library search directories.
    target_link_directories(gk3 PRIVATE
        Libraries/ffmpeg/lib/mac
        Libraries/fmod/lib/mac
        Libraries/GLEW/lib/mac
        Libraries/libpng/mac/lib
        Libraries/zlib/mac/lib
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
        png                                         # libpng
        z                                           # zlib
        ${SDL2}                                     # SDL
        ${COREFOUNDATION_LIB}                       # Core Foundation
        ${OPENGL_LIB}                               # OpenGL
    )

    # Copy libraries after build.
    set(LIB_DEST "${MAC_APP_CONTENTS}/Libraries")
    set(LIB_FFMPEG_SRC "${LIBS_SRC}/ffmpeg/lib/mac")
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libavcodec.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libavformat.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libavutil.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libswresample.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libswscale.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/fmod/lib/mac/libfmod.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/GLEW/lib/mac/libGLEW.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/libpng/mac/lib/libpng.dylib" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/zlib/mac/lib/libz.dylib" "${LIB_DEST}"
    )

else() # Linux
    target_include_directories(gk3 PRIVATE
        Libraries/libpng/linux/include
        Libraries/SDL/linux/include
        Libraries/zlib/linux/include
    )

    # Specify library search directories.
    target_link_directories(gk3 PRIVATE
        Libraries/ffmpeg/lib/linux
        Libraries/fmod/lib/linux
        Libraries/GLEW/lib/linux
        Libraries/libpng/linux/lib
        Libraries/SDL/linux/lib
        Libraries/zlib/linux/lib
    )

    # Link all the libraries.
    target_link_libraries(gk3
        avcodec avformat avutil swresample swscale  # ffmpeg
        fmod                                        # fmod
        GLEW                                        # GLEW
        png                                         # libpng
        z                                           # zlib
        SDL2                                        # SDL
        GL                                          # OpenGL
        ${CMAKE_DL_LIBS}                            # dlopen/dlclose
        pthread                                     # pthread
    )

    # Copy libraries post-build.
    set(LIB_DEST "${PLAT_BINARY_DIR}")
    set(LIB_FFMPEG_SRC "${LIBS_SRC}/ffmpeg/lib/linux")
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ln -fs "${PLAT_SOURCE_DIR}/Assets" "${PLAT_BINARY_DIR}"  #TODO: Can maybe use ${CMAKE_COMMAND} -E create_symlink?
        COMMAND ln -fs "${PLAT_SOURCE_DIR}/Data" "${PLAT_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libavcodec.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libavformat.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libavutil.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libswresample.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/libswscale.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/fmod/lib/linux/libfmod.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/GLEW/lib/linux/libGLEW.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/libpng/linux/lib/libpng.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/zlib/linux/lib/libz.so" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/SDL/linux/lib/libSDL2.so" "${LIB_DEST}"
        VERBATIM
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

##################
# PLATFORM CONFIG
##################
# Here, we mostly have misc configs required to get the engine working on a specific platform, or to help with debugging.
if(WIN32)
    # Use repo root as debugger working directory.
    # This allows running the game without copying Assets/Data to the Build folder.
    set_property(TARGET gk3 PROPERTY
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # Set GK3 as the startup project.
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gk3)

    # Add App.rc as a source to set the Windows app icon.
    set(APP_RESOURCES
         Publish/Windows/App.rc
    )
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${APP_RESOURCES})
    target_sources(gk3 PRIVATE ${APP_RESOURCES})

elseif(APPLE)
    # Build a Mac app (rather than command line tool).
    set_target_properties(gk3 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Publish/Mac/Info.plist
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}

        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
        XCODE_ATTRIBUTE_PRODUCT_NAME "Gabriel Knight 3"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.kromenak.gengine"
        XCODE_ATTRIBUTE_MARKETING_VERSION ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION 1
    )

    # Add xcassets file under "Resources" in project (required for AppIcon to work correctly).
    target_sources(gk3 PRIVATE Publish/Mac/Assets.xcassets)
    set_source_files_properties(Publish/Mac/Assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

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
    # NOTE: CLion doesn't yet have a WORKING_DIRECTORY CMake variable.

    # Non-Xcode IDEs (like CLion) do not seem to automatically copy frameworks. So, do that manually.
    # They also don't have a way to set working directory, so symbolic link Assets/Data folders to avoid copying.
    if(NOT XCODE)
        set(MAC_APP_FRAMEWORKS "${MAC_APP_CONTENTS}/Frameworks")
        add_custom_command(TARGET gk3
            POST_BUILD
            COMMAND mkdir -p "${MAC_APP_FRAMEWORKS}"
            COMMAND cp -R "${PLAT_SOURCE_DIR}/Libraries/SDL/SDL2.framework" "${MAC_APP_FRAMEWORKS}"
            COMMAND ln -fs "${PLAT_SOURCE_DIR}/Assets" "${MAC_APP_CONTENTS}/Resources"
            COMMAND ln -fs "${PLAT_SOURCE_DIR}/Data" "${MAC_APP_CONTENTS}/Resources"
        )
    endif()
else() # Linux
    # Add symbolic links to Assets/Data so the game runs even if working directory isn't the source root.
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ln -fs "${PLAT_SOURCE_DIR}/Assets" "${PLAT_BINARY_DIR}"  #TODO: Can maybe use ${CMAKE_COMMAND} -E create_symlink?
        COMMAND ln -fs "${PLAT_SOURCE_DIR}/Data" "${PLAT_BINARY_DIR}"
    )
endif()

# Create deploy target, which creates a final release of the game.
# This packages the built game into a zip file in the "Bin" directory, with any copyrighted material excluded.
add_custom_target(deploy DEPENDS "deploy_output")
if(WIN32)
    set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    set(DEPLOY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Bin/$<CONFIG>")
    set(ZIP_FILE_NAME "GK3-Win-${PROJECT_VERSION}.zip")

    add_custom_command(
        OUTPUT "deploy_output_first"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DEPLOY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${BUILD_DIR}" "${DEPLOY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rename "${DEPLOY_DIR}/gk3.exe" "${DEPLOY_DIR}/GK3.exe"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ROOT_DIR}/Assets" "${DEPLOY_DIR}/Assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}/Data"
        COMMAND ${CMAKE_COMMAND} -E copy "${ROOT_DIR}/Data/README.md" "${DEPLOY_DIR}/Data"
        COMMAND ${CMAKE_COMMAND} -E copy "${ROOT_DIR}/GK3.ini" "${DEPLOY_DIR}"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "deploy_output"
        DEPENDS "deploy_output_first"
        COMMAND ${CMAKE_COMMAND} -E tar cvf "${ROOT_DIR}/${ZIP_FILE_NAME}" --format=zip .
        COMMAND ${CMAKE_COMMAND} -E rename "${ROOT_DIR}/${ZIP_FILE_NAME}" "${DEPLOY_DIR}/${ZIP_FILE_NAME}"
        WORKING_DIRECTORY "${DEPLOY_DIR}"
        VERBATIM
    )
elseif(APPLE)
    add_custom_command(
        OUTPUT "deploy_output"
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Publish/Deploy.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> ${CMAKE_CURRENT_SOURCE_DIR}/Bin/$<CONFIG> ${PROJECT_VERSION} Mac
        VERBATIM
    )
else() # Linux
    add_custom_command(
        OUTPUT "deploy_output"
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Publish/Deploy.sh ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Bin/$<CONFIG> ${PROJECT_VERSION} Linux
        VERBATIM
    )
endif()
add_dependencies(deploy gk3)

# Add tests subdirectory (creates the "tests" target).
add_subdirectory(Tests)