#!/bin/bash
# Builds ffmpeg for use with G-Engine.

# Get rid of previous build output.
rm -rf out

# On Windows, MSYS2 shell must be used. See README for more details.
UNAME="$(uname)"
if [ "${UNAME:0:7}" == "MSYS_NT" ]; then
	TOOLCHAIN="--toolchain=msvc"
fi

# Configure ffmpeg for G-Engine usage. Here's what we're doing:
# - Create shared library optimized for size using the desired toolchain.
# - Completely disable a lot of features we aren't using: docs, avdevice, avfilter, network, debug, programs, indevs, outdevs, encoders, muxers, etc.
# - Disable all protocols except for file. We only need to read video from files in this game.
# - Disable all demuxers except bink and AVI. MPEG/MP3 are also needed for Russian localization, which encodes videos differently.
# - Disable all decoders except bink, AVI, and MPEG/MP3.
# - Disable all parsers except those required by AVI and MPEG/MP3.
SHARED_CONFIGURE_ARGS="--prefix=out --disable-static --enable-shared --enable-small ${TOOLCHAIN} \
--disable-doc --disable-avdevice --disable-avfilter --disable-network --disable-debug --disable-programs --disable-indevs --disable-outdevs --disable-filters --disable-encoders --disable-muxers --disable-hwaccels --disable-bsfs \
--disable-protocols --enable-protocol=file \
--disable-demuxers --enable-demuxer=bink,avi,mpegps,mpegvideo,mp3 \
--disable-decoders --enable-decoder=bink,binkaudio_rdft,msrle,pcm_s16le,cinepak,mpeg1video,mpeg2video,mp1,mp2,mp3 \
--disable-parsers --enable-parser=mpegvideo,mpegaudio,mpeg4video,pcm"

if [ ${UNAME} == "Darwin" ]; then
	# Build for Intel Macs.
	make distclean
	./configure --enable-cross-compile --arch=x86_64 --target-os=darwin --cc='clang -arch x86_64' --host-cc='clang' ${SHARED_CONFIGURE_ARGS}
	make -j$(($(sysctl -n hw.ncpu) - 1))
	make install
	mv out/lib out/lib_x64

	# Build for Apple Silicon Macs.
	make distclean
	./configure --enable-cross-compile --arch=arm64 --target-os=darwin --cc='clang -arch arm64' --host-cc='clang' ${SHARED_CONFIGURE_ARGS}
	make -j$(($(sysctl -n hw.ncpu) - 1))
	make install
	mv out/lib out/lib_arm64 

	# Combine both architectures into one set of files.
	mkdir out/lib
	lipo -create -output out/lib/libavcodec.dylib out/lib_x64/libavcodec.dylib out/lib_arm64/libavcodec.dylib
	lipo -create -output out/lib/libavformat.dylib out/lib_x64/libavformat.dylib out/lib_arm64/libavformat.dylib
	lipo -create -output out/lib/libavutil.dylib out/lib_x64/libavutil.dylib out/lib_arm64/libavutil.dylib
	lipo -create -output out/lib/libswresample.dylib out/lib_x64/libswresample.dylib out/lib_arm64/libswresample.dylib
	lipo -create -output out/lib/libswscale.dylib out/lib_x64/libswscale.dylib out/lib_arm64/libswscale.dylib

	# On Mac, we've got to set library LC_ID_DYLIB and LC_LOAD_DYLIB commands correctly.
	# Modify LC_LOAD_DYLIB for dependent libraries to use @rpath to find the dependent library.
	# This allows the EXE to specify where to search for these libraries in its "Runtime Search Paths" value.
	install_name_tool -change out/lib/libavutil.60.dylib @rpath/libavutil.dylib out/lib/libavcodec.dylib
	install_name_tool -change out/lib/libavutil.60.dylib @rpath/libavutil.dylib out/lib/libavformat.dylib
	install_name_tool -change out/lib/libavcodec.62.dylib @rpath/libavcodec.dylib out/lib/libavformat.dylib
	install_name_tool -change out/lib/libavutil.60.dylib @rpath/libavutil.dylib out/lib/libswresample.dylib
	install_name_tool -change out/lib/libavutil.60.dylib @rpath/libavutil.dylib out/lib/libswscale.dylib

	# Change ID for all dylibs to use @rpath.
	# When built against, these IDs will be used by executable as path to load from.
	install_name_tool -id @rpath/libavcodec.dylib out/lib/libavcodec.dylib
	install_name_tool -id @rpath/libavformat.dylib out/lib/libavformat.dylib
	install_name_tool -id @rpath/libavutil.dylib out/lib/libavutil.dylib
	install_name_tool -id @rpath/libswresample.dylib out/lib/libswresample.dylib
	install_name_tool -id @rpath/libswscale.dylib out/lib/libswscale.dylib
else
	# On Linux and Windows, just make and install.
	./configure ${SHARED_CONFIGURE_ARGS}
	make -j$(($(nproc) - 1))
	make install
fi