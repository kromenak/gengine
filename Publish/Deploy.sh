#!/bin/sh
ROOT_DIR=$1
BUILD_DIR=$2
DEPLOY_DIR=$3
VERSION_NUM=$4
PLATFORM=$5

# Data validation
if [ -z "${ROOT_DIR}" ]; then
    exit 1;
fi
if [ -z "${BUILD_DIR}" ]; then
    exit 1;
fi
if [ -z "${DEPLOY_DIR}" ]; then
    exit 1;
fi

# Make sure install directory (and intermediates) exist and is empty.
mkdir -p "${DEPLOY_DIR}"
rm -rf "${DEPLOY_DIR}"/*

if [ ${PLATFORM} = "Windows" ]; then
    # Copy over EXE.
    cp "${BUILD_DIR}/gk3.exe" "${DEPLOY_DIR}/GK3.exe"

    # Copy over all DLLs.
    cp "${BUILD_DIR}/"*.dll "${DEPLOY_DIR}"

    # Copy Assets folder.
    cp -r "${ROOT_DIR}/Assets" "${DEPLOY_DIR}"

    # Create a Data directory with README file.
    mkdir "${DEPLOY_DIR}/Data"
    cp "${ROOT_DIR}/Data/README.md" "${DEPLOY_DIR}/Data/"

    # Copy over ini file.
    cp "${ROOT_DIR}/GK3.ini" "${DEPLOY_DIR}"

    # Create zip.
    cd "${DEPLOY_DIR}"
    zip -r GK3-Win-${VERSION_NUM}.zip .
fi

if [ ${PLATFORM} = "Mac" ]; then
    APP_FILENAME="Gabriel Knight 3.app"

    # Copy Assets to app bundle.
    ASSETS_DIR=${BUILD_DIR}/${APP_FILENAME}/Contents/Resources
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
    cp "${BUILD_DIR}/gk3" "${DEPLOY_DIR}/${APP_FILENAME}"

    # Replace the rpath of the application.
    # The "$ORIGIN" keyword uses the path of the executable at runtime.
    patchelf --set-rpath '$ORIGIN' "${DEPLOY_DIR}/${APP_FILENAME}"

    # Copy over all the libraries.
    cp "${BUILD_DIR}/"*.so "${DEPLOY_DIR}"

    # Create zip file for distribution.
    cd "${DEPLOY_DIR}"
    FINAL_ZIP_NAME="GK3-Linux-${VERSION_NUM}.zip"
    zip -r ${FINAL_ZIP_NAME} .
fi

