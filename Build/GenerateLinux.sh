#!/bin/sh

# Determine the config to generate, falling back on Debug if none is specified.
CONFIG=Debug
if [ $# -gt 0 ]; then
	CONFIG=$1 
fi
echo "Generating Linux makefiles with config ${CONFIG}."

# Generate the CMake project.
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
cmake -S .. -B Linux_${CONFIG} -DCMAKE_BUILD_TYPE=${CONFIG}