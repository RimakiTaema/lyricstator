# Building Lyricstator for Android

## Prerequisites

1. **Android Studio** with NDK support
2. **SDL2 Android Libraries** - Download from libsdl.org
3. **Android SDK** (API level 21+)
4. **Android NDK** (r21 or newer)

## Setup Steps

1. **Download SDL2 Android Libraries:**
   ```bash
   # Download SDL2, SDL2_mixer, SDL2_ttf, SDL2_image Android builds
   # Extract .so files to android/app/src/main/jniLibs/[abi]/
   ```

2. **Copy Assets:**
   ```bash
   cp -r assets android/app/src/main/assets/
   cp -r samples android/app/src/main/assets/
   ```

3. **Build with Android Studio:**
   - Open the `android/` folder in Android Studio
   - Sync Gradle files
   - Build APK or AAB

## Key Differences from Desktop

- **Touch Input**: X key becomes touch gesture for resource pack GUI
- **Asset Loading**: Uses Android APK asset system
- **Permissions**: Requires microphone permission for pitch detection
- **Screen**: Optimized for landscape orientation
- **Storage**: Uses Android external storage for user files

## Controls

- **Touch**: Navigate menus and select options
- **Long Press**: Open resource pack GUI (replaces X key)
- **Pinch**: Zoom lyrics display
- **Swipe**: Navigate between songs

## Performance Notes

- Minimum Android 5.0 (API 21)
- Requires 2GB+ RAM for optimal performance
- Hardware audio processing recommended
- OpenGL ES 2.0+ for smooth graphics
