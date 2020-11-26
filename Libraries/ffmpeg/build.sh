#!/bin/sh
# Builds ffmpeg for use with G-Engine.

# Get rid of previous build output.
rm -rf out

# On Windows (compiling with MinGW or MSYS2 or something like that), use MSVC toolchain.
# "uname" returns "Darwin" on MacOS, "Linux" on Linux, and various values on Windows (depending on Shell used).
UNAME="$(uname)"
if [ "$(expr substr ${UNAME} 1 7)" == "MSYS_NT" ]; then
TOOLCHAIN="--toolchain=msvc"
fi

# Configure for G-Engine usage. We want shared libraries, only demuxing/decoding (playback only) from files.
# Only a couple file formats are needed (bink, avi) with a could decoders (bink, cinepak, msrle, pcm).
./configure --prefix=out --disable-static --enable-shared --enable-small ${TOOLCHAIN} \
			--disable-doc --disable-avdevice --disable-avfilter --disable-postproc --disable-network --disable-debug \
			--disable-encoders --disable-muxers --disable-hwaccels --disable-parsers --disable-bsfs --disable-indevs --disable-outdevs --disable-filters \
			--disable-decoders --enable-decoder=bink --enable-decoder=binkaudio_rdft --enable-decoder=msrle --enable-decoder=pcm_s16le --enable-decoder=cinepak \
			--disable-demuxers --enable-demuxer=bink --enable-demuxer=avi \
			--disable-protocols --enable-protocol=file

# Make libraries. "install" puts libs in out/bin directory.
make
make install

# On Mac, final dylibs are located in "out/lib"
# On Windows, final dlls are located in "out/bin" (shrug)

# On Mac, we've got to set library LC_ID_DYLIB and LC_LOAD_DYLIB commands correctly.
if [ UNAME == "Darwin" ]; then
	# Modify LC_LOAD_DYLIB for dependent libraries to use @rpath to find the dependent library.
	# This allows the EXE to specify where to search for these libraries in its "Runtime Search Paths" value.
	install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libavcodec.dylib
	install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libavformat.dylib
	install_name_tool -change out/lib/libavcodec.58.dylib @rpath/libavcodec.dylib out/lib/libavformat.dylib
	install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libswresample.dylib
	install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libswscale.dylib

	# Change ID for all dylibs to use @rpath.
	# When built against, these IDs will be used by executable as path to load from.
	install_name_tool -id @rpath/libavcodec.dylib out/lib/libavcodec.dylib
	install_name_tool -id @rpath/libavformat.dylib out/lib/libavformat.dylib
	install_name_tool -id @rpath/libavutil.dylib out/lib/libavutil.dylib
	install_name_tool -id @rpath/libswresample.dylib out/lib/libswresample.dylib
	install_name_tool -id @rpath/libswscale.dylib out/lib/libswscale.dylib
fi