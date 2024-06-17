# Test files are just all the cpp files in this directory.
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

# Also add the catch.hh header file so it appears in the IDE.
list(APPEND TEST_SOURCES "catch.hh")

# Add tests executable.
add_executable(tests ${TEST_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

# Tests have selective dependencies on GK3 sources and headers.
# For example, if a test is testing AABBs, the test EXE needs the AABB header and source.
# Likely I could structure my code differently to make this cleaner/more modular...but this'll do for now.

# Header locations.
target_include_directories(tests PRIVATE
    ../Source
    ../Source/Audio
    ../Source/Barn
    ../Source/Containers
    ../Source/Debug
    ../Source/GK3
    ../Source/GK3/Scene
    ../Source/IO
    ../Source/Math
    ../Source/Memory
    ../Source/Platform
    ../Source/Primitives
    ../Source/Rendering
    ../Source/RTTI
    ../Source/Sheep
    ../Source/Util
    ../Source/Video
)

# Game source files being tested.
target_sources(tests PRIVATE
    ../Source/GK3/Timeblock.cpp

    ../Source/IO/BinaryReader.cpp
    ../Source/IO/BinaryWriter.cpp
    ../Source/IO/mstream.cpp

    ../Source/Math/Matrix3.cpp
    ../Source/Math/Matrix4.cpp
    ../Source/Math/Quaternion.cpp
    ../Source/Math/Vector2.cpp
    ../Source/Math/Vector3.cpp
    ../Source/Math/Vector4.cpp

    ../Source/Memory/LinearAllocator.cpp
    ../Source/Memory/StackAllocator.cpp
    ../Source/Memory/FreestyleAllocator.cpp

    ../Source/Primitives/AABB.cpp
    ../Source/Primitives/Collisions.cpp
    ../Source/Primitives/Line.cpp
    ../Source/Primitives/LineSegment.cpp
    ../Source/Primitives/Plane.cpp
    ../Source/Primitives/Rect.cpp
    ../Source/Primitives/RectUtil.cpp
    ../Source/Primitives/Sphere.cpp
    ../Source/Primitives/Triangle.cpp

    ../Source/RTTI/TypeInfo.cpp
)
