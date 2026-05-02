# FlowPlusPlus

A visual flowchart interpreter built with C++ and Qt 6, following the
structure of [Crafting Interpreters](https://craftinginterpreters.com/)
by Robert Nystrom.

## Concept

Instead of typing code, you build programs visually using flowchart nodes
connected by arrows. FlowPlusPlus walks the graph and executes the logic.

## Node Types

| Shape | Meaning |
|---|---|
| Rounded rectangle | Start / Stop |
| Rectangle | Process (assignment, expression) |
| Diamond | Decision (if/else branch) |
| Parallelogram | Input / Output |

## Tech Stack

- **C++17**
- **Qt 6.5+** (Widgets, QGraphicsScene)
- **CMake + Ninja**
- Developed on Fedora Linux with Qt Creator

## Build

```bash
cmake -B build -G Ninja
cmake --build build
./build/FlowPlusPlus
```

## Progress (Crafting Interpreters parallel)

- [x] Chapter 1 — Project setup, canvas window, toolbar
- [x] Chapter 2 — FlowNode base class, FlowConnection
- [x] Chapter 3 — Concrete nodes (StartStop, Process, Decision, IO)
- [x] Chapter 4 — Placing nodes on canvas from toolbar
- [x] Chapter 5 — Connecting nodes with arrows
- [x] Chapter 6 — Value & Environment (variables)
- [x] Chapter 7 — Interpreter: walking the graph
- [x] Chapter 8 — Control flow (Decision node branching)
- [x] Chapter 9 — I/O nodes (print, read input)

##### can't wait to make a releasable version :)

### next steps to come
- ✅ Core interpreter
- ✅ Save/Load
- ✅ Minimap
- ⬜ Multi-select + copy/paste
- ⬜ Undo/Redo
- ⬜ Sample .fpp files
- ⬜ AppImage packaging (Linux)
- ⬜ Windows installer
- ⬜ macOS DMG
- ⬜ GitHub Actions CI/CD
