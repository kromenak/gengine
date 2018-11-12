#!/bin/sh

mkdir $TARGET_BUILD_DIR/$FULL_PRODUCT_NAME/Contents/Libs
cp "$SRCROOT"/../Libraries/GLEW/lib/mac/*.dylib $TARGET_BUILD_DIR/$FULL_PRODUCT_NAME/Contents/Libs
cp "$SRCROOT"/../Libraries/zlib/lib/mac/*.dylib $TARGET_BUILD_DIR/$FULL_PRODUCT_NAME/Contents/Libs
cp "$SRCROOT"/../Libraries/fmod/lib/*.dylib $TARGET_BUILD_DIR/$FULL_PRODUCT_NAME/Contents/Libs
