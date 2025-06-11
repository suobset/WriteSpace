#!/bin/bash

echo "Installing WriteSpace..."

# Check for dependencies
command -v pandoc >/dev/null 2>&1 || { echo >&2 "Pandoc is required but not installed. Aborting."; exit 1; }
command -v pdflatex >/dev/null 2>&1 || { echo >&2 "A LaTeX distribution (like TeXLive) is required but not installed. Aborting."; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo >&2 "CMake is required but not installed. Aborting."; exit 1; }
command -v make >/dev/null 2>&1 || { echo >&2 "make is required but not installed. Aborting."; exit 1; }
(command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1) || { echo >&2 "A C++ compiler (g++ or clang++) is required but not installed. Aborting."; exit 1; }


# Install the main script
echo "Installing 'writespace' command to /usr/local/bin..."
chmod +x writespace
sudo cp writespace /usr/local/bin/writespace

# Build and install the man page
echo "Installing man page..."
# Ensure docs directory and man page source exist before attempting to build/install
if [ ! -f docs/writespace.1.md ]; then
    echo >&2 "Man page source (docs/writespace.1.md) not found. Skipping man page installation."
else
    # Check if docs/writespace.1 exists, if not, or if .md is newer, regenerate
    if [ ! -f docs/writespace.1 ] || [ docs/writespace.1.md -nt docs/writespace.1 ]; then
        echo "Generating man page from Markdown source..."
        pandoc -s -f markdown -t man docs/writespace.1.md -o docs/writespace.1
        if [ $? -ne 0 ]; then
            echo >&2 "Failed to generate man page. Skipping man page installation."
        else
            sudo mkdir -p /usr/local/share/man/man1
            sudo cp docs/writespace.1 /usr/local/share/man/man1/writespace.1
            echo "Man page installed to /usr/local/share/man/man1/writespace.1"
        fi
    else
        echo "Man page (docs/writespace.1) is up-to-date. Installing existing one."
        sudo mkdir -p /usr/local/share/man/man1
        sudo cp docs/writespace.1 /usr/local/share/man/man1/writespace.1
        echo "Man page installed to /usr/local/share/man/man1/writespace.1"
    fi
fi

# Configure CMake (default CMAKE_INSTALL_PREFIX is /usr/local)
cmake . -DCMAKE_BUILD_TYPE=Release
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

echo ""
echo "✓ WriteSpace (CLI and TUI) installed successfully!"
echo "Run 'writespace help' or 'writespace-tui' to get started."
