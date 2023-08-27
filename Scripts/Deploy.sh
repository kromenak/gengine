#!/bin/sh
ROOT_DIR=$1
BUILD_DIR=$2
DEPLOY_DIR=$3
VERSION_NUM=$4
PLATFORM=$5

# Make sure install directory (and intermediates) exist and is empty.
mkdir -p "${DEPLOY_DIR}"
rm -rf "${DEPLOY_DIR}"/*

if [ ${PLATFORM} = "Mac" ]; then
	APP_FILENAME="Gabriel Knight 3.app"

	# Copy Assets to app bundle.
	ASSETS_DIR=${BUILD_DIR}/${APP_FILENAME}/Contents/Resources
	echo ${ASSETS_DIR}
	cp -r ${ROOT_DIR}/Assets "${ASSETS_DIR}"

	# Make Data directory in app bundle with README file.
	mkdir -p "${ASSETS_DIR}/Data"
	cp ${ROOT_DIR}/Data/README.md "${ASSETS_DIR}/Data/"

	# Copy over .ini file.
	cp ${ROOT_DIR}/GK3.ini "${ASSETS_DIR}"

	# Copy the app bundle to the deploy directory.
	cp -r "${BUILD_DIR}/${APP_FILENAME}" "${DEPLOY_DIR}"

	# Create zip archive containing app.
	cd "${DEPLOY_DIR}"
	zip -r GK3-Mac-${VERSION_NUM}.zip "Gabriel Knight 3.app"
fi

if [ ${PLATFORM} = "Linux" ]; then
	APP_FILENAME="Gabriel Knight 3"

	# Copy Assets to deploy directory.
	cp -r "${ROOT_DIR}/Assets" "${DEPLOY_DIR}"

	# Make Data directory in app bundle with README file.
	mkdir -p "${DEPLOY_DIR}/Data"
	cp "${ROOT_DIR}/Data/README.md" "${DEPLOY_DIR}/Data/"

	# Copy over .ini file.
	cp "${ROOT_DIR}/GK3.ini" "${DEPLOY_DIR}"

	# Copy the application (and rename to be player-facing).
	cp "${BUILD_DIR}/gk3" "${DEPLOY_DIR}/Gabriel Knight 3"

	# Replace the rpath of the application.
	# The "$ORIGIN" keyword uses the path of the executable at runtime.
	patchelf --set-rpath '$ORIGIN' "${DEPLOY_DIR}/Gabriel Knight 3"

	# Copy over all the libraries.
	cp "${ROOT_DIR}/Libraries/ffmpeg/lib/linux/libavcodec.so" "${DEPLOY_DIR}"
	cp "${ROOT_DIR}/Libraries/ffmpeg/lib/linux/libavformat.so" "${DEPLOY_DIR}"
	cp "${ROOT_DIR}/Libraries/ffmpeg/lib/linux/libavutil.so" "${DEPLOY_DIR}"
	cp "${ROOT_DIR}/Libraries/ffmpeg/lib/linux/libswresample.so" "${DEPLOY_DIR}"
	cp "${ROOT_DIR}/Libraries/ffmpeg/lib/linux/libswscale.so" "${DEPLOY_DIR}"

	cp "${ROOT_DIR}/Libraries/fmod/lib/linux/libfmod.so" "${DEPLOY_DIR}"
	cp "${ROOT_DIR}/Libraries/GLEW/lib/linux/libGLEW.so" "${DEPLOY_DIR}"
	cp "${ROOT_DIR}/Libraries/zlib/lib/linux/libz.so" "${DEPLOY_DIR}"
	cp "${ROOT_DIR}/Libraries/SDL/linux/lib/libSDL2.so" "${DEPLOY_DIR}"

	# Create a copy of the deploy directory called "Gabriel Knight 3".
	# This is needed so that unzipping the zip file creates a folder with the proper name.
	FINAL_ZIP_SRC="${DEPLOY_DIR}/../Gabriel Knight 3"
	rm -rf "${FINAL_ZIP_SRC}"
	cp -r "${DEPLOY_DIR}" "${FINAL_ZIP_SRC}"
	
	# CD over to the zip folder root and generate the zip file.
	cd "${FINAL_ZIP_SRC}"
	FINAL_ZIP_NAME="GK3-Linux-${VERSION_NUM}.zip"
	zip -r ${FINAL_ZIP_NAME} "../Gabriel Knight 3"

	# Move the zip file back to the deploy directory (that's where appveyor expects it for artifacting).
	# Also delete the temporary zip src directory.
	cp ${FINAL_ZIP_NAME} ${DEPLOY_DIR}
	rm -rf "${FINAL_ZIP_SRC}"
fi

