#!/bin/bash

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
