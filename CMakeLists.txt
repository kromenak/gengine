cmake_minimum_required(VERSION 3.17)

project(gengine VERSION 0.0.5
                DESCRIPTION "Gabriel Knight 3 game engine"
                LANGUAGES C CXX)

# Build for C++14, standard required, no extensions.
# Using global "set" rather thant target-specific commands b/c I want all my targets to use the same rules for these.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For Xcode, generate schemes for targets. This ensures targets appear in target dropdown.
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

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

# Add main executable - the game.
add_executable(gk3 ${source_files} ${asset_files})

# Add header locations.
# For any folder listed here, you can add headers in that folder without providing a path.
target_include_directories(gk3 PRIVATE
    Source
    Source/Assets
    Source/Audio
    Source/Containers
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

# Warning/error settings.
if(MSVC)
    #target_compile_options(gk3 PRIVATE /W3 /wd4065)
    target_compile_options(gk3 PRIVATE /wd26812)
else()
    #target_compile_options(gk3 PRIVATE -Wall -Wextra -pedantic)
endif()

# Other compiler flags
if(MSVC)
    target_compile_options(gk3 PRIVATE 
        /MP     # Multithreaded compiling - it's faster
    )
endif()

# Library locations/linking/copying.
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
elseif(APPLE)
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
    # Copies libraries to app bundle post-build.
    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ${PLAT_SOURCE_DIR}/Scripts/PostBuild.sh ${PLAT_SOURCE_DIR} app
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
        COMMAND ${PLAT_SOURCE_DIR}/Scripts/Deploy.sh ${PLAT_SOURCE_DIR} ${PLAT_BINARY_DIR}/$<CONFIG> ${PLAT_SOURCE_DIR}/Bin/$<CONFIG> ${PROJECT_VERSION}
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

# Add tests subdirectory (creates the "tests" target).
add_subdirectory(Tests)
