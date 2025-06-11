#!/bin/bash
# Configure CMake (default CMAKE_INSTALL_PREFIX is /usr/local)
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo >&2 "CMake configuration failed for C++ TUI. Aborting TUI installation."
    cd ..
    # rm -rf build_cpp_tui # Optional: clean up build dir on failure
    exit 1
fi

# Build
make -j$(nproc) # Build in parallel
if [ $? -ne 0 ]; then
    echo >&2 "C++ TUI build failed. Aborting TUI installation."
    cd ..
    # rm -rf build_cpp_tui
    exit 1
fi

# Install
sudo make install
if [ $? -ne 0 ]; then
    echo >&2 "C++ TUI installation failed."
    cd ..
    # rm -rf build_cpp_tui
    exit 1
fi

cd ..
# rm -rf build_cpp_tui # Clean up build directory after successful install (optional, good for CI)
echo "✓ WriteSpace TUI (C++) installed successfully as writespace-tui."

