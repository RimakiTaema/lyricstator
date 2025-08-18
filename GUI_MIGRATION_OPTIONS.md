# GUI Framework Migration Options for Lyricstator

## Option 1: TGUI + SDL2 (Recommended)
**Pros:**
- Minimal migration effort
- Modern widget system
- Keeps proven SDL2 multimedia capabilities
- Cross-platform including Android

**Migration Steps:**
1. Add TGUI dependency to CMakeLists.txt
2. Replace custom GUI with TGUI widgets
3. Keep existing SDL2 audio/rendering code

## Option 2: SDL3 Migration
**Pros:**
- Future-proof, officially stable
- Better performance
- Improved API design
- SDL2 compatibility layer available

**Migration Steps:**
1. Update CMakeLists.txt for SDL3
2. Use SDL2-compat layer initially
3. Gradually migrate to SDL3 API

## Option 3: Brisk Framework
**Pros:**
- Modern C++20 features
- GPU-accelerated rendering
- Excellent multimedia support

**Cons:**
- Significant rewrite required
- Newer framework (less mature)

## Recommendation
Start with **TGUI + SDL2** for immediate improvements, then consider SDL3 migration later.
