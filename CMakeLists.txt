cmake_minimum_required(VERSION 3.16)
project(WriteSpaceTUI_CPP VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Fetch FTXUI using FetchContent ---
include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git # Corrected repo name casing if needed
    GIT_TAG v6.1.9 # Updated version
)
FetchContent_MakeAvailable(ftxui)

# --- Add TUI source files ---
add_executable(writespace-tui src/main.cpp)

# --- Link FTXUI ---
target_link_libraries(writespace-tui PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# --- Install rule ---
# This will install to /usr/local/bin by default when CMAKE_INSTALL_PREFIX is /usr/local
install(TARGETS writespace-tui DESTINATION bin)
