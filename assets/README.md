# Lyricstator Assets

This directory contains all the assets used by the Lyricstator application.

## Directory Structure

\`\`\`
assets/
├── fonts/           # Font files for text rendering
├── images/          # Background images and UI elements
├── audio/           # Sample audio files for testing
└── samples/         # Sample .lystr files
\`\`\`

## Font Requirements

The application expects the following fonts:
- `fonts/default.ttf` - Main UI font
- `fonts/lyrics.ttf` - Lyrics display font
- `fonts/bold.ttf` - Bold text font

## Image Assets

- `images/background.png` - Default background image
- `images/note_indicator.png` - Pitch indicator graphics

## Adding New Assets

1. Place files in the appropriate subdirectory
2. Update the AssetManager to load them
3. Reference them in your code using AssetManager::getInstance().getAssetPath()
