#!/bin/sh
# Builds ffmpeg for use with G-Engine.

# Get rid of previous build output.
rm -rf out

# Configure for G-Engine usage. We want shared libraries, only demuxing/decoding (playback only) from files.
# Only a couple file formats are needed (bink, avi)
./configure --prefix=out --disable-static --enable-shared --enable-small \
			--disable-doc --disable-avdevice --disable-avfilter --disable-postproc --disable-network --disable-debug \
			--disable-encoders --disable-muxers --disable-hwaccels --disable-parsers --disable-bsfs --disable-indevs --disable-outdevs --disable-filters \
			--disable-decoders --enable-decoder=bink --enable-decoder=binkaudio_rdft --enable-decoder=msrle --enable-decoder=pcm_s16le --enable-decoder=cinepak \
			--disable-demuxers --enable-demuxer=bink --enable-demuxer=avi \
			--disable-protocols --enable-protocol=file

# Make libraries. "install" puts libs in out/bin directory.
make
make install

# Get rid of unneeded stuff.
rm -rf out/bin
rm -rf out/lib/pkgconfig
rm -rf out/share

# Modify LC_LOAD_DYLIB for dependent libraries to use @rpath to find the dependent library.
# This allows us to just stick the dylibs next to the EXE for distribution. 
install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libavcodec.dylib
install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libavformat.dylib
install_name_tool -change out/lib/libavcodec.58.dylib @rpath/libavcodec.dylib out/lib/libavformat.dylib
install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libswresample.dylib
install_name_tool -change out/lib/libavutil.56.dylib @rpath/libavutil.dylib out/lib/libswscale.dylib

# Change ID for all dylibs to use @rpath. This allows EXE to reference files next to it in same directory.
install_name_tool -id @rpath/libavcodec.dylib out/lib/libavcodec.dylib
install_name_tool -id @rpath/libavformat.dylib out/lib/libavformat.dylib
install_name_tool -id @rpath/libavutil.dylib out/lib/libavutil.dylib
install_name_tool -id @rpath/libswresample.dylib out/lib/libswresample.dylib
install_name_tool -id @rpath/libswscale.dylib out/lib/libswscale.dylib

