# Test files are just all the cpp files in this directory.
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

# Also add the catch.hh header file so it appears in the IDE.
list(APPEND TEST_SOURCES "catch.hh")

# Add tests executable.
add_executable(tests ${TEST_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

# This define allows some test sources to know whether they are being compiled for the actual game or for tests.
# Meant to help us avoid pulling too many dependencies into the test executable.
target_compile_definitions(tests PRIVATE TESTS)

# Tests have selective dependencies on GK3 sources and headers.
# For example, if a test is testing AABBs, the test EXE needs the AABB header and source.
# Likely I could structure my code differently to make this cleaner/more modular...but this'll do for now.

# Header locations.
target_include_directories(tests PRIVATE
    ../Source
    ../Source/Engine/Audio
    ../Source/Engine/Containers
    ../Source/Engine/Debug
    ../Source/GK3
    ../Source/GK3/Scene
    ../Source/Engine/IO
    ../Source/Engine/Math
    ../Source/Engine/Memory
    ../Source/Engine/Platform
    ../Source/Engine/Primitives
    ../Source/Engine/Rendering
    ../Source/Engine/RTTI
    ../Source/Engine/Sheep
    ../Source/Engine/Util
    ../Source/Engine/Video
)

# Game source files being tested.
target_sources(tests PRIVATE
    ../Source/GK3/Timeblock.cpp

    ../Source/Engine/IO/BinaryReader.cpp
    ../Source/Engine/IO/BinaryWriter.cpp
    ../Source/Engine/IO/mstream.cpp

    ../Source/Engine/Math/Matrix3.cpp
    ../Source/Engine/Math/Matrix4.cpp
    ../Source/Engine/Math/Quaternion.cpp
    ../Source/Engine/Math/Vector2.cpp
    ../Source/Engine/Math/Vector3.cpp
    ../Source/Engine/Math/Vector4.cpp

    ../Source/Engine/Memory/LinearAllocator.cpp
    ../Source/Engine/Memory/StackAllocator.cpp
    ../Source/Engine/Memory/FreestyleAllocator.cpp

    ../Source/Engine/Primitives/AABB.cpp
    ../Source/Engine/Primitives/Collisions.cpp
    ../Source/Engine/Primitives/Line.cpp
    ../Source/Engine/Primitives/LineSegment.cpp
    ../Source/Engine/Primitives/Plane.cpp
    ../Source/Engine/Primitives/Rect.cpp
    ../Source/Engine/Primitives/RectUtil.cpp
    ../Source/Engine/Primitives/Sphere.cpp
    ../Source/Engine/Primitives/Triangle.cpp

    ../Source/Engine/RTTI/TypeInfo.cpp
)