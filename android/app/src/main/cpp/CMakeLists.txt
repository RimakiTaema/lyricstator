cmake_minimum_required(VERSION 3.22.1)
project(lyricstator-android)

# Import SDL2 libraries
find_library(log-lib log)
find_library(android-lib android)

# Add SDL2 as imported libraries
add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libSDL2.so)

add_library(SDL2_mixer SHARED IMPORTED)
set_target_properties(SDL2_mixer PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libSDL2_mixer.so)

add_library(SDL2_ttf SHARED IMPORTED)
set_target_properties(SDL2_ttf PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libSDL2_ttf.so)

add_library(SDL2_image SHARED IMPORTED)
set_target_properties(SDL2_image PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libSDL2_image.so)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../SDL2/include)

# Source files (same as desktop version)
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../src/*.cpp")

# Create shared library for Android
add_library(lyricstator SHARED ${SOURCES})

# Link libraries
target_link_libraries(lyricstator
    SDL2
    SDL2_mixer
    SDL2_ttf
    SDL2_image
    ${log-lib}
    ${android-lib}
)

# Android-specific compile definitions
target_compile_definitions(lyricstator PRIVATE
    ANDROID_BUILD=1
    ASSETS_PATH="/android_asset/"
)
