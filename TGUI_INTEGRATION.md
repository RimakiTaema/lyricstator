# TGUI Integration Guide

## Overview
Lyricstator now uses TGUI for a modern, maintainable GUI system that's easy to understand and extend.

## Key Benefits
- **Simple**: Clean widget-based architecture
- **Beautiful**: Modern styling with smooth animations  
- **Maintainable**: Clear separation of concerns
- **Extensible**: Easy to add new features

## Architecture

### TGUIKaraokeDisplay
- Main karaoke interface with lyrics and pitch visualization
- Simple methods: `SetLyric()`, `HighlightLyric()`, `UpdatePitch()`
- Easy theme switching with `ApplyTheme()`

### TGUIResourcePackGUI  
- Resource pack selector with list and preview
- Toggle with `Show()/Hide()/Toggle()`
- Automatic pack discovery and loading

## Usage Example
\`\`\`cpp
// Initialize TGUI
tgui::Gui gui(window);

// Create displays
TGUIKaraokeDisplay karaokeDisplay;
TGUIResourcePackGUI resourceGUI;

// Simple setup
karaokeDisplay.Initialize(gui);
resourceGUI.Initialize(gui, &assetManager);

// Easy updates
karaokeDisplay.SetLyric("Hello World");
karaokeDisplay.HighlightLyric(0.5f);
resourceGUI.Toggle(); // Show/hide with X key
\`\`\`

## Building
Requires TGUI library:
\`\`\`bash
# Ubuntu/Debian
sudo apt install libtgui-dev

# Build
mkdir build && cd build
cmake ..
make
\`\`\`

## Customization
- Themes in `ApplyTheme()` method
- Colors easily changed in constructors
- Layout adjustable in `SetupLayout()` methods
- New widgets simple to add
