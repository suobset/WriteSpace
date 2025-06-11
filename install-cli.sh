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

echo "Run 'writespace help' to get started."
