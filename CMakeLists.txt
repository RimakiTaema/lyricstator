cmake_minimum_required(VERSION 3.16)
project(Lyricstator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/audio/AudioManager.cpp
    src/audio/MidiParser.cpp
    src/ai/NoteDetector.cpp
    src/scripting/LystrParser.cpp
    src/scripting/LystrInterpreter.cpp
    src/gui/Window.cpp
    src/gui/KaraokeDisplay.cpp
    src/gui/UserInterface.cpp
    src/sync/SynchronizationManager.cpp
    src/export/FormatExporter.cpp
    src/utils/ErrorHandler.cpp
    src/utils/FileUtils.cpp
)

# Header files
set(HEADERS
    src/core/Application.h
    src/audio/AudioManager.h
    src/audio/MidiParser.h
    src/ai/NoteDetector.h
    src/scripting/LystrParser.h
    src/scripting/LystrInterpreter.h
    src/gui/Window.h
    src/gui/KaraokeDisplay.h
    src/gui/UserInterface.h
    src/sync/SynchronizationManager.h
    src/export/FormatExporter.h
    src/utils/ErrorHandler.h
    src/utils/FileUtils.h
    src/common/Types.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    pthread
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY samples DESTINATION ${CMAKE_BINARY_DIR})

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets DESTINATION share/lyricstator)
install(DIRECTORY samples DESTINATION share/lyricstator)