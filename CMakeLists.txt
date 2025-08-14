cmake_minimum_required(VERSION 3.16)
project(Lyricstator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
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

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Optional: Find additional libraries for enhanced features
find_package(Threads REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(src)

# Source files organized by component
set(CORE_SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/AssetManager.cpp
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
    src/gui/ResourcePackGUI.cpp  # Added ResourcePackGUI source
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

# Header files
set(HEADERS
    src/core/Application.h
    src/core/AssetManager.h
    src/audio/AudioManager.h
    src/audio/MidiParser.h
    src/ai/NoteDetector.h
    src/scripting/LystrParser.h
    src/scripting/LystrInterpreter.h
    src/gui/Window.h
    src/gui/KaraokeDisplay.h
    src/gui/UserInterface.h
    src/gui/ResourcePackGUI.h  # Added ResourcePackGUI header
    src/sync/SynchronizationManager.h
    src/export/FormatExporter.h
    src/utils/ErrorHandler.h
    src/utils/FileUtils.h
    src/common/Types.h
)

# Create executable
add_executable(${PROJECT_NAME} ${ALL_SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    Threads::Threads
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/fonts)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/images)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/audio)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/samples)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/themes)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/icons)  # Added icons directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/resource_packs)  # Added resource packs directory

# Copy assets to build directory
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/samples)
    file(COPY ${CMAKE_SOURCE_DIR}/samples DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets DESTINATION share/lyricstator OPTIONAL)
install(DIRECTORY samples DESTINATION share/lyricstator OPTIONAL)

if(UNIX AND NOT APPLE)
    install(FILES assets/icons/lyricstator.png 
            DESTINATION share/lyricstator/icons OPTIONAL)
    install(FILES assets/icons/lyricstator-48.png 
            DESTINATION share/icons/hicolor/48x48/apps 
            RENAME lyricstator.png OPTIONAL)
    install(FILES assets/icons/lyricstator-32.png 
            DESTINATION share/icons/hicolor/32x32/apps 
            RENAME lyricstator.png OPTIONAL)
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "Lyricstator")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced Karaoke and Lyric Visualization System")
set(CPACK_PACKAGE_VENDOR "Lyricstator Team")

include(CPack)

# Print build information
message(STATUS "Lyricstator ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
