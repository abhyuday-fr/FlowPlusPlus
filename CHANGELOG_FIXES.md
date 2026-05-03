# FlowPlusPlus - Update & Fix Report (v1.0.6)

This report summarizes the critical issues identified and resolved to improve the stability and cross-platform performance of FlowPlusPlus.

---

## 1. Resolved Issues

### 🔴 Undo/Redo Crashes (Memory Corruption)
- **Issue:** The application would "terminate abnormally" (crash) when performing Undo or Redo operations, especially after deleting nodes or moving them.
- **Root Cause:** The `UndoCommands` were using a "Serialization" approach. When a node was deleted, the command destroyed the object. When undone, it recreated a *new* object from JSON. This invalidated all raw pointers held by other commands (like `MoveNodeCommand`), leading to "dangling pointers" and segmentation faults.
- **The Fix:** Refactored the entire Undo system to use **Pointer-based Ownership**. 
    - Objects are no longer destroyed immediately. 
    - Instead, they are removed from the Scene and held by the Command object.
    - If the command is purged from the stack, it cleans up the memory; otherwise, the pointer remains valid and stable across all operations.

### 🔴 Windows Rendering "Marks" (Graphical Artifacts)
- **Issue:** Moving nodes on Windows left ghosting artifacts or "marks" on the canvas.
- **Root Cause:** Inefficient viewport update modes and issues with how the default raster engine handles transparency/redrawing on certain Windows graphics drivers.
- **The Fix:** 
    - Forced the use of `QOpenGLWidget` as the viewport for Windows.
    - Switched to `SmartViewportUpdate` for more intelligent redrawing.
    - Enabled `CacheBackground` to improve performance and prevent redraw flickering.

### 🔴 Redo Shortcut (Ctrl+Y) Failure
- **Issue:** The Redo operation worked via the menu but the `Ctrl+Y` shortcut was unresponsive on some Windows systems.
- **Root Cause:** Conflict or lack of explicit registration for the platform-specific "Redo" key sequence.
- **The Fix:** Explicitly registered both `Ctrl+Shift+Z` (Standard) and `Ctrl+Y` (Windows standard) as shortcuts for the Redo action and added a ToolTip for user guidance.

---

## 2. Technical Updates & Improvements

### 🛠 Build System (CMakeLists.txt)
- **Cleanup:** Reorganized the CMake file to follow modern standards.
- **Windows Support:** Corrected the linking of `Qt6::OpenGL` and `Qt6::OpenGLWidgets` to ensure the application builds and runs correctly on Windows.
- **Standardization:** Ensured the executable target is defined before setting properties or linking libraries.

### 🧪 Canvas & Scene Logic
- **Undo Stack Hardening:** Added logic to `FlowScene::clearAll()` to clear the Undo Stack whenever a new file is created or a sample is loaded. This prevents the "ghost history" problem where you could undo into a file that no longer exists.
- **Performance:** Optimized `MoveNodeCommand` to only push to the undo stack upon **Mouse Release**, preventing the stack from being flooded with hundreds of incremental move events during a single drag.
- **Connection Stability:** Added `attach()` and `detach()` methods to `FlowConnection` to safely manage relationship links between nodes during undo/redo cycles without losing data.

### 📝 Miscellaneous
- **Version Sync:** Updated `main.cpp` and `CMakeLists.txt` to consistently reflect version `1.0.4`.
- **Typo Fix:** Corrected `haserro()` to `hasError()` in the node API.

---

## 3. CI/CD & Deployment
- **GitHub Actions:** Verified and updated `.github/workflows/release.yml`.
- **New Release:** Successfully tagged and triggered workflow `v1.0.6` to generate new binaries for Linux (Flatpak), Windows (ZIP), and macOS (DMG).
