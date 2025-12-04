#!/bin/bash
set -e

echo "=========================================="
echo "Building Native Chess Library for macOS"
echo "=========================================="

# Directories
PROJECT_ROOT="${SOURCE_ROOT}/../"
NATIVE_DIR="${PROJECT_ROOT}/native"
BUILD_DIR="${NATIVE_DIR}/build/macos_flutter"
DEST_DIR="${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}"

echo "Project root: ${PROJECT_ROOT}"
echo "Native dir: ${NATIVE_DIR}"
echo "Build dir: ${BUILD_DIR}"
echo "Destination: ${DEST_DIR}"
echo ""

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure CMake
echo "Configuring CMake..."
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="${ARCHS}" \
    ../..

# Build
echo "Building chess_bridge library..."
cmake --build . --target chess_bridge --config Release

# Verify library was built
if [ ! -f "libchess_bridge.dylib" ]; then
    echo "ERROR: libchess_bridge.dylib was not created!"
    exit 1
fi

echo "Library built successfully!"

# Copy to app bundle
echo "Copying library to app bundle..."
mkdir -p "${DEST_DIR}"
cp -v libchess_bridge.dylib "${DEST_DIR}/"

# Sign the library (required for macOS app)
echo "Signing library..."
codesign --force --sign - "${DEST_DIR}/libchess_bridge.dylib" 2>/dev/null || true

echo ""
echo "=========================================="
echo "✓ Native library built and bundled!"
echo "=========================================="

