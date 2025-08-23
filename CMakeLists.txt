cmake_minimum_required(VERSION 3.16)
project(Lyricstator_Qt6 VERSION 1.0.0 LANGUAGES CXX)

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
# Qt6 Configuration
# ------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ------------------------------
# Include FetchContent for non-Qt dependencies
# ------------------------------
include(FetchContent)

# JSONCPP
FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.6
    GIT_SHALLOW TRUE
)

# Make dependencies available
FetchContent_MakeAvailable(jsoncpp)

# ------------------------------
# Source Files
# ------------------------------
set(CORE_SOURCES
    src/core/ConfigManager.cpp
    src/core/LyricParser.cpp
    src/core/MidiParser.cpp
    src/core/SongManager.cpp
)

set(AUDIO_SOURCES
    # Audio sources removed - will be implemented with Qt6 audio
)

set(QT_GUI_SOURCES
    src/gui/qt/QtMainWindow.cpp
    src/gui/qt/QtKaraokeDisplay.cpp
    src/gui/qt/QtResourcePackGUI.cpp
    src/gui/qt/QtSongBrowser.cpp
    src/gui/qt/QtEqualizer.cpp
    src/gui/qt/QtKeybindEditor.cpp
)

set(UTILS_SOURCES
    src/utils/FileUtils.cpp
    src/utils/Logger.cpp
    src/utils/StringUtils.cpp
)

set(AI_SOURCES
    src/ai/PitchAnalyzer.cpp
)

set(EXPORT_SOURCES
    src/export/LyricExporter.cpp
    src/export/AudioExporter.cpp
)

set(SCRIPTING_SOURCES
    src/scripting/ScriptEngine.cpp
)

set(SYNC_SOURCES
    src/sync/LyricSynchronizer.cpp
)

# ------------------------------
# Header Files
# ------------------------------
set(CORE_HEADERS
    src/core/ConfigManager.h
    src/core/LyricParser.h
    src/core/MidiParser.h
    src/core/SongManager.h
)

set(AUDIO_HEADERS
    # Audio headers removed - will be implemented with Qt6 audio
)

set(QT_GUI_HEADERS
    src/gui/qt/QtMainWindow.h
    src/gui/qt/QtKaraokeDisplay.h
    src/gui/qt/QtResourcePackGUI.h
    src/gui/qt/QtSongBrowser.h
    src/gui/qt/QtEqualizer.h
    src/gui/qt/QtKeybindEditor.h
)

set(UTILS_HEADERS
    src/utils/FileUtils.h
    src/utils/Logger.h
    src/utils/StringUtils.h
)

set(AI_HEADERS
    src/ai/PitchAnalyzer.h
)

set(EXPORT_HEADERS
    src/export/LyricExporter.h
    src/export/AudioExporter.h
)

set(SCRIPTING_HEADERS
    src/scripting/ScriptEngine.h
)

set(SYNC_HEADERS
    src/sync/LyricSynchronizer.h
)

# ------------------------------
# Resources
# ------------------------------
set(RESOURCES
    resources/resources.qrc
)

# ------------------------------
# Combine all sources and headers
# ------------------------------
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${AUDIO_SOURCES}
    ${QT_GUI_SOURCES}
    ${UTILS_SOURCES}
    ${AI_SOURCES}
    ${EXPORT_SOURCES}
    ${SCRIPTING_SOURCES}
    ${SYNC_SOURCES}
)

set(ALL_HEADERS
    ${CORE_HEADERS}
    ${AUDIO_HEADERS}
    ${QT_GUI_HEADERS}
    ${UTILS_HEADERS}
    ${AI_HEADERS}
    ${EXPORT_HEADERS}
    ${SCRIPTING_HEADERS}
    ${SYNC_HEADERS}
)

# ------------------------------
# Create executable
# ------------------------------
if(ANDROID)
    add_library(Lyricstator_Qt6 SHARED ${ALL_SOURCES} ${ALL_HEADERS} ${RESOURCES})
else()
    add_executable(Lyricstator_Qt6 ${ALL_SOURCES} ${ALL_HEADERS} ${RESOURCES})
endif()

# ------------------------------
# Include directories
# ------------------------------
target_include_directories(Lyricstator_Qt6 PRIVATE
    src
    src/core
    src/audio
    src/gui
    src/gui/qt
    src/utils
    src/ai
    src/export
    src/scripting
    src/sync
)

# ------------------------------
# Link libraries
# ------------------------------
target_link_libraries(Lyricstator_Qt6
    Qt6::Core
    Qt6::Widgets
    jsoncpp_lib
)

# ------------------------------
# Platform-specific configurations
# ------------------------------

# Android configuration
if(ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS AndroidExtras)
    target_link_libraries(Lyricstator_Qt6 Qt6::AndroidExtras)
    
    # Set Android package properties
    set_target_properties(Lyricstator_Qt6 PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main
        QT_ANDROID_ABIS "armeabi-v7a;arm64-v8a"
    )
endif()

# Windows configuration
if(WIN32)
    # Set Windows executable properties
    set_target_properties(Lyricstator_Qt6 PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# macOS configuration
if(APPLE)
    set_target_properties(Lyricstator_Qt6 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )
endif()

# Linux configuration
if(UNIX AND NOT APPLE AND NOT ANDROID)
    # Install desktop file
    install(FILES lyricstator.desktop.in
        DESTINATION share/applications
        RENAME lyricstator.desktop
    )
    
    # Install icon
    install(FILES assets/icon.png
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME lyricstator.png
    )
endif()

# ------------------------------
# Compiler flags
# ------------------------------
target_compile_features(Lyricstator_Qt6 PRIVATE cxx_std_17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Lyricstator_Qt6 PRIVATE DEBUG)
    target_compile_options(Lyricstator_Qt6 PRIVATE -g -O0)
else()
    target_compile_definitions(Lyricstator_Qt6 PRIVATE NDEBUG)
    target_compile_options(Lyricstator_Qt6 PRIVATE -O3)
endif()

# ------------------------------
# Installation
# ------------------------------
install(TARGETS Lyricstator_Qt6
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# ------------------------------
# Qt Installer Framework
# ------------------------------
find_package(Qt6 QUIET COMPONENTS InstallerFramework)
if(Qt6InstallerFramework_FOUND)
    add_custom_target(installer
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/create_installer.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/installer
        COMMENT "Creating installer project"
    )
    
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/create_installer.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/installer/create_installer.sh
        @ONLY
    )
endif()

# ------------------------------
# Build Summary
# ------------------------------
message(STATUS "=== Lyricstator Qt6 Build Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt6 Components: Core, Widgets")
message(STATUS "Target: Lyricstator_Qt6")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
if(ANDROID)
    message(STATUS "Android ABI: ${ANDROID_ABI}")
endif()
message(STATUS "==========================================")
