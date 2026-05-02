cmake_minimum_required(VERSION 3.19)
project(FlowPlusPlus LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# ── Sources ──────────────────────────────────────────────
set(FPP_SOURCES
    src/main.cpp

    # UI / Main Window
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui

    # Canvas  (QGraphicsScene / QGraphicsView layer)
    src/canvas/FlowScene.cpp
    src/canvas/FlowScene.h
    src/canvas/FlowView.cpp
    src/canvas/FlowView.h
    src/canvas/FlowConnection.cpp
    src/canvas/FlowConnection.h
    src/canvas/MiniMap.cpp
    src/canvas/MiniMap.h
    src/canvas/UndoCommands.cpp
    src/canvas/UndoCommands.h

    # Nodes  (our visual AST nodes)
    src/nodes/FlowNode.cpp
    src/nodes/FlowNode.h
    src/nodes/StartStopNode.cpp
    src/nodes/StartStopNode.h
    src/nodes/ProcessNode.cpp
    src/nodes/ProcessNode.h
    src/nodes/IONode.cpp
    src/nodes/IONode.h
    src/nodes/DecisionNode.cpp
    src/nodes/DecisionNode.h

    # Interpreter core  (Crafting Interpreters style)
    src/interpreter/Value.cpp
    src/interpreter/Value.h
    src/interpreter/Environment.cpp
    src/interpreter/Environment.h
    src/interpreter/Interpreter.cpp
    src/interpreter/Interpreter.h
)

qt_add_executable(FlowPlusPlus ${FPP_SOURCES}
    src/canvas/MiniMap.h
    src/canvas/MiniMap.cpp
    src/canvas/UndoCommands.h
    src/canvas/UndoCommands.cpp)

# ── Include dirs ─────────────────────────────────────────
target_include_directories(FlowPlusPlus PRIVATE
    src
    src/canvas
    src/nodes
    src/interpreter
    src/ui
)

# ── Link ─────────────────────────────────────────────────
target_link_libraries(FlowPlusPlus PRIVATE
    Qt6::Core
    Qt6::Widgets
)

# ── C++ standard ─────────────────────────────────────────
target_compile_features(FlowPlusPlus PRIVATE cxx_std_17)

include(GNUInstallDirs)
install(TARGETS FlowPlusPlus
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET FlowPlusPlus
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
