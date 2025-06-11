#!/bin/bash

echo "Uninstalling WriteSpace..."

# Ensure the script is run with root privileges, as it's removing system files
if [ "$(id -u)" -ne 0 ]; then
  echo "This script must be run with sudo or as the root user." >&2
  exit 1
fi

# Remove the main script from /usr/local/bin
echo "Removing 'writespace' command..."
if [ -f /usr/local/bin/writespace ]; then
    sudo rm /usr/local/bin/writespace # Added sudo
    echo "✓ 'writespace' command removed from /usr/local/bin."
else
    echo "i 'writespace' command not found in /usr/local/bin. Skipping."
fi

# Remove the C++ TUI executable from /usr/local/bin
# This assumes the CMake install prefix was /usr/local and target name was writespace-tui
echo "Removing 'writespace-tui' (C++ TUI) command..."
if [ -f /usr/local/bin/writespace-tui ]; then
    sudo rm /usr/local/bin/writespace-tui
    echo "✓ 'writespace-tui' command removed from /usr/local/bin."
else
    echo "i 'writespace-tui' command not found in /usr/local/bin. Skipping."
fi
# Also attempt removal of writespace-tui-cpp if it was named that way previously
if [ -f /usr/local/bin/writespace-tui-cpp ]; then
    sudo rm /usr/local/bin/writespace-tui-cpp
    echo "✓ Old 'writespace-tui-cpp' command removed from /usr/local/bin."
fi


# Remove the man page from /usr/local/share/man/man1
echo "Removing man page..."
if [ -f /usr/local/share/man/man1/writespace.1 ]; then
    sudo rm /usr/local/share/man/man1/writespace.1 # Added sudo
    echo "✓ Man page removed from /usr/local/share/man/man1."

    # It's good practice to update the man page database after removal
    if command -v mandb >/dev/null 2>&1; then
        echo "Updating man page database..."
        sudo mandb # Added sudo
    fi
else
    echo "i Man page not found in /usr/local/share/man/man1. Skipping."
fi

# Remove old Python TUI related files from the repository (if they exist)
# These are not system files, so don't need sudo, but uninstall script might be run from anywhere.
# Better to let user manage repo files, or do this via make clean if defined.
# For now, just notifying.
echo "Note: Old Python TUI files (writespace_tui.py, writespace_tui.css, requirements.txt) are no longer used."
echo "You may remove them from your repository manually if they are still present."

# Remove build directory if it exists (common practice for CMake)
if [ -d "build_cpp_tui" ]; then
    echo "Removing C++ TUI build directory 'build_cpp_tui'..."
    rm -rf build_cpp_tui
fi
if [ -f "docs/writespace.1" ]; then # remove generated man page file from repo
    echo "Removing generated man page 'docs/writespace.1' from repository..."
    rm -f docs/writespace.1
fi


echo ""
echo "✓ WriteSpace (CLI and TUI components) uninstallation process complete."
echo "Note: C++ build dependencies (like FTXUI, if downloaded by CMake) and system libraries (cmake, make, g++) are not removed."
echo "      If you installed Python packages via pip for the old TUI, they are also not removed."
