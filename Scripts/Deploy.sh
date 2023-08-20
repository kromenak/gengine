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

	cp "${BUILD_DIR}/gk3" "${DEPLOY_DIR}/Gabriel Knight 3"

	#zip -r GK3-Linux-${VERSION_NUM}.zip ${DEPLOY_DIR}/*
fi

