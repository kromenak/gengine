#!/bin/sh
SOURCE_DIR=$1
BUILD_DIR=$2
DEPLOY_DIR=$3
VERSION_NUM=$4

# Gonna just hardcode this for now...
APP_FILENAME="Gabriel Knight 3.app"

# Make sure install directory (and intermediates) exist and is empty.
mkdir -p "${DEPLOY_DIR}"
rm -rf "${DEPLOY_DIR}"/*

# Copy Assets to app bundle.
ASSETS_DIR=${BUILD_DIR}/${APP_FILENAME}/Contents/Resources
echo ${ASSETS_DIR}
cp -r ${SOURCE_DIR}/Assets "${ASSETS_DIR}"

# Make Data directory in app bundle with README file.
mkdir -p "${ASSETS_DIR}/Data"
cp ${SOURCE_DIR}/Data/README.md "${ASSETS_DIR}/Data/"

# Copy over .ini file.
cp ${SOURCE_DIR}/GK3.ini "${ASSETS_DIR}"

# Copy the app bundle to the deploy directory.
cp -r "${BUILD_DIR}/${APP_FILENAME}" "${DEPLOY_DIR}"

# Create zip archive containing app.
cd "${DEPLOY_DIR}"
zip -r GK3-Mac-${VERSION_NUM}.zip "Gabriel Knight 3.app"