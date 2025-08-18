cmake_minimum_required(VERSION 3.16)
project(Lyricstator VERSION 1.0.0 LANGUAGES CXX)

# ------------------------------
# C++ Standard
# ------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------
# Build type
# ------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ------------------------------
# Include FetchContent
# ------------------------------
include(FetchContent)

# ------------------------------
# SDL2
# ------------------------------
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.0
    GIT_SHALLOW TRUE
)

# SDL2_mixer
FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-2.8.0
    GIT_SHALLOW TRUE
)

# SDL2_ttf
FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.22.0
    GIT_SHALLOW TRUE
)

# SDL2_image
FetchContent_Declare(
    SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-2.8.2
    GIT_SHALLOW TRUE
)

# ------------------------------
# SFML (required for TGUI)
# ------------------------------
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW TRUE
)

# ------------------------------
# TGUI
# ------------------------------
if(DEFINED ENV{TGUI_VERSION})
    set(TGUI_VERSION_DEFAULT $ENV{TGUI_VERSION})
else()
    set(TGUI_VERSION_DEFAULT "v1.10.0")
endif()

FetchContent_Declare(
    TGUI
    GIT_REPOSITORY https://github.com/texus/TGUI.git
    GIT_TAG ${TGUI_VERSION_DEFAULT}
    GIT_SHALLOW TRUE
)

# ------------------------------
# JSONCPP
# ------------------------------
FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.6
    GIT_SHALLOW TRUE
)

# ------------------------------
# Make dependencies available
# ------------------------------
FetchContent_MakeAvailable(SDL2 SDL2_mixer SDL2_ttf SDL2_image SFML TGUI jsoncpp)

# ------------------------------
# Compiler options
# ------------------------------
if(MSVC)
    add_compile_options(/W4 /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
    else()
        add_compile_options(-g -O0)
    endif()
endif()

# ------------------------------
# Threads
# ------------------------------
find_package(Threads REQUIRED)

# ------------------------------
# Include directories
# ------------------------------
include_directories(src)

# ------------------------------
# Source files
# ------------------------------
set(CORE_SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/AssetManager.cpp
    src/core/SettingsManager.cpp
)

set(AUDIO_SOURCES
    src/audio/AudioManager.cpp
    src/audio/MidiParser.cpp
)

set(AI_SOURCES
    src/ai/NoteDetector.cpp
)

set(SCRIPTING_SOURCES
    src/scripting/LystrParser.cpp
    src/scripting/LystrInterpreter.cpp
)

set(GUI_SOURCES
    src/gui/Window.cpp
    src/gui/KaraokeDisplay.cpp
    src/gui/UserInterface.cpp
    src/gui/ResourcePackGUI.cpp
    src/gui/TGUIKaraokeDisplay.cpp
    src/gui/TGUIResourcePackGUI.cpp
    src/gui/TGUISongBrowser.cpp
    src/gui/TGUIEqualizer.cpp
    src/gui/TGUIKeybindEditor.cpp
)

set(SYNC_SOURCES
    src/sync/SynchronizationManager.cpp
)

set(EXPORT_SOURCES
    src/export/FormatExporter.cpp
)

set(UTILS_SOURCES
    src/utils/ErrorHandler.cpp
    src/utils/FileUtils.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${AUDIO_SOURCES}
    ${AI_SOURCES}
    ${SCRIPTING_SOURCES}
    ${GUI_SOURCES}
    ${SYNC_SOURCES}
    ${EXPORT_SOURCES}
    ${UTILS_SOURCES}
)

# ------------------------------
# Executable
# ------------------------------
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

target_link_libraries(${PROJECT_NAME} 
    SDL2::SDL2
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
    SDL2_image::SDL2_image
    tgui
    sfml-graphics
    sfml-window
    sfml-system
    jsoncpp_lib
    Threads::Threads
)

# ------------------------------
# Assets
# ------------------------------
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/fonts)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/images)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/audio)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/samples)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/themes)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/icons)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/resource_packs)

if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/samples)
    file(COPY ${CMAKE_SOURCE_DIR}/samples DESTINATION ${CMAKE_BINARY_DIR})
endif()

# ------------------------------
# Install
# ------------------------------
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets DESTINATION share/lyricstator OPTIONAL)
install(DIRECTORY samples DESTINATION share/lyricstator OPTIONAL)

# ------------------------------
# Linux desktop integration
# ------------------------------
if(UNIX AND NOT APPLE)
    configure_file(
        ${CMAKE_SOURCE_DIR}/lyricstator.desktop.in
        ${CMAKE_BINARY_DIR}/lyricstator.desktop
        @ONLY
    )
    install(FILES ${CMAKE_BINARY_DIR}/lyricstator.desktop 
            DESTINATION share/applications)
    
    install(FILES assets/icons/lyricstator.png 
            DESTINATION share/lyricstator/icons OPTIONAL)
    install(FILES assets/icons/lyricstator-48.png 
            DESTINATION share/icons/hicolor/48x48/apps 
            RENAME lyricstator.png OPTIONAL)
    install(FILES assets/icons/lyricstator-32.png 
            DESTINATION share/icons/hicolor/32x32/apps 
            RENAME lyricstator.png OPTIONAL)
endif()

# ------------------------------
# Packaging
# ------------------------------
set(CPACK_PACKAGE_NAME "Lyricstator")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced Karaoke and Lyric Visualization System")
set(CPACK_PACKAGE_VENDOR "Lyricstator Team")
include(CPack)

# ------------------------------
# Build info
# ------------------------------
message(STATUS "Lyricstator ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "TGUI Version: ${TGUI_VERSION_DEFAULT}")
message(STATUS "Using FetchContent for automatic dependency management")
