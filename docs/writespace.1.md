% WRITESPACE(1) | General Commands Manual
% The WriteSpace Team
% August 2025

# NAME

writespace, writespace-tui - command-line and text-based user interface writing environment.

# SYNOPSIS

**writespace** <command> [options]
**writespace-tui**

# DESCRIPTION

**WriteSpace** provides a suite of tools for managing writing projects using Markdown, Git, and Pandoc. It offers both a command-line interface (`writespace`) for specific operations and a comprehensive Text-based User Interface (`writespace-tui`) for an integrated writing experience.

**writespace**
: The command-line tool allows for scripting and direct execution of core project management tasks such as creating new projects and building them into various formats.

**writespace-tui**
: A keyboard-driven, multi-pane Text-based User Interface (TUI) application built in C++ using the FTXUI library. It provides an immersive environment for writing and project management. The TUI layout includes:
    *   **Header:** Displays application title, current project (placeholder), open file name, and editor status (placeholders for line/column numbers).
    *   **Project Browser (Left Pane):** Shows a tree view of project files from `01_drafts` and `02_templates` directories. Git status indicators (e.g., [M], [A], [?]) are displayed next to files, updated asynchronously.
    *   **Editor Pane (Right, Main Area):** Currently a read-only viewer for text files. Basic line-by-line editing is supported via a modal dialog (`e` key).
    *   **Info Panel (Bottom-Left Pane):** A tabbed area providing contextual information.
        *   **Outline Tab:** Shows a document outline generated from Markdown headers in the currently active editor.
        *   **Stats Tab:** (Placeholder) Intended for text statistics.
        *   **Citations Tab:** (Placeholder) Intended for citation management.
    *   **Status & Output Log (Bottom-Right Pane):** Displays messages, errors, and real-time output from build commands or Git operations.
    *   **Footer:** Shows keybindings and other contextual help.

Key TUI features include:
    *   **Command Palette (`Ctrl+P`):** Allows quick access to various commands, including project builds (e.g., "Build: pdf", "Build: html" via Makefile, asynchronous with output) and Git operations (e.g., "Git: Show Status Output", "Git: Refresh Status").
    *   **Dialogs:**
        *   **New Project (`n` in Project Browser):** Prompts for project name and template, then creates the project structure.
        *   **Delete Item (`d` in Project Browser):** Prompts for confirmation before deleting a file or directory.
    *   **Quit (`Ctrl+Q`):** Exits the `writespace-tui` application.

# TUI USAGE

Launch the Text-based User Interface using the command:
```
writespace-tui
```

The TUI is primarily keyboard-driven. Here are some basic keybindings:

*   **`Ctrl+Q`**: Quit the application.
*   **`Ctrl+P`**: Open the Command Palette. From here, you can type to filter and select commands such as:
    *   `Build: pdf` (and other formats like `html`, `tex`)
    *   `Git: Show Status Output`, `Git: Refresh Status`
*   **Project Browser:**
    *   **Arrow Keys**: Navigate the list of files and directories.
    *   **`Enter`**: Opens the selected file in the Editor Pane (read-only view). (Directory selection is logged but no action yet).
    *   **`n`**: Opens the "New Project" dialog.
    *   **`d`**: Opens the "Delete Item" confirmation dialog for the selected file/directory.
*   **Editor Pane (File Viewer):**
    *   **Arrow Keys (Up/Down)**: Navigates the active line (highlighted) for editing.
    *   **`e`**: Opens a modal dialog to edit the currently highlighted line.
*   **Line Edit Modal:**
    *   **`Enter`**: Saves the changes to the line and closes the dialog.
    *   **`Escape`**: Closes the dialog, discarding changes to the line.
*   **Dialogs (New Project, Delete Item, Command Palette):**
    *   **`Escape`**: Closes the active dialog.
    *   **Arrow Keys/Enter**: Navigate and select options within dialogs.

# BUILDING THE TUI (for Developers)

The `writespace-tui` application is built using CMake and requires a C++17 compliant compiler (e.g., g++ or clang++), Make, and CMake itself. The FTXUI library is fetched automatically by CMake during the configuration step.
Users should use the provided `install.sh` script for installation, which handles the build process.

# COMMANDS (CLI)

This section refers to the `writespace` command-line interface.

**new** *project-name* **--template** *template-name*
: Creates a new project in `01_drafts/` based on a specified template from `02_templates/`.

**build** *format*
: Run from within a project directory (e.g., `01_drafts/my_project/`). Builds the project's main Markdown file into a specified format. Valid formats include `html`, `html-plain`, `pdf`, `pdf-ieee`, `pdf-mla`, and `tex`.

**list-templates**
: Displays a list of available project templates from `02_templates/`.

# FILES

**/01_drafts/**
: Location of all active writing projects. Each subdirectory within `01_drafts` is typically a separate project.

**/02_templates/**
: Contains all Markdown (.md), CSS (.css), and LaTeX (.tex) templates used for creating new projects.

**/library.bib**
: The central BibTeX bibliography file intended for use across all projects.

**Makefile**
: Located in the repository root, this file defines the build rules used by the `writespace build` command and the TUI's build system integration.

**CMakeLists.txt**
: The main CMake build configuration file for `writespace-tui` (the C++ TUI).

**src/main.cpp**
: The primary C++ source file for the `writespace-tui` application.

**docs/writespace.1**
: The compiled man page file. The source is `docs/writespace.1.md`.

# AUTHOR

Written by The WriteSpace Team.
Further contributions and development by the AI Software Engineering Agent.

# SEE ALSO

**pandoc**(1), **git**(1), **make**(1), **cmake**(1), **ftxui**(C++ library)
