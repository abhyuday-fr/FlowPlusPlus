cmake_minimum_required(VERSION 3.19)

# ── Version ───────────────────────────────────────────────
set(FPP_VERSION_MAJOR 1)
set(FPP_VERSION_MINOR 0)
set(FPP_VERSION_PATCH 4)
set(FPP_VERSION "${FPP_VERSION_MAJOR}.${FPP_VERSION_MINOR}.${FPP_VERSION_PATCH}")

project(FlowPlusPlus VERSION ${FPP_VERSION} LANGUAGES CXX)

# Find Qt
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

if(WIN32)
    find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)
endif()

qt_standard_project_setup()

# ── Sources ───────────────────────────────────────────────
set(FPP_SOURCES
    src/main.cpp

    # UI / Main Window
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui

    # Canvas (QGraphicsScene / QGraphicsView layer)
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

    # Nodes (our visual AST nodes)
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

    # Interpreter core (Crafting Interpreters style)
    src/interpreter/Value.cpp
    src/interpreter/Value.h
    src/interpreter/Environment.cpp
    src/interpreter/Environment.h
    src/interpreter/Interpreter.cpp
    src/interpreter/Interpreter.h
)

qt_add_executable(FlowPlusPlus ${FPP_SOURCES})

# ── Sample resources ──────────────────────────────────────
qt_add_resources(FlowPlusPlus "samples"
    PREFIX "/samples"
    BASE "${CMAKE_SOURCE_DIR}/samples"
    FILES
        samples/hello_world.fpp
        samples/calculator.fpp
        samples/countdown.fpp
        samples/even_odd.fpp
        samples/fibonacci.fpp
        samples/grade_checker.fpp
)

# ── Include dirs ──────────────────────────────────────────
target_include_directories(FlowPlusPlus PRIVATE
    src
    src/canvas
    src/nodes
    src/interpreter
    src/ui
)

# ── Link ──────────────────────────────────────────────────
target_link_libraries(FlowPlusPlus PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Windows Specifics
if(WIN32)
    target_link_libraries(FlowPlusPlus PRIVATE 
        Qt6::OpenGL 
        Qt6::OpenGLWidgets
    )
    target_compile_definitions(FlowPlusPlus PRIVATE 
        QT_OPENGL_DYNAMIC
        QT_OPENGL_ES_2_ANGLE
    )
    set_target_properties(FlowPlusPlus PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# ── C++ standard ──────────────────────────────────────────
target_compile_features(FlowPlusPlus PRIVATE cxx_std_17)

# ── Platform-specific properties ──────────────────────────
if(APPLE)
    set_target_properties(FlowPlusPlus PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME        "FlowPlusPlus"
        MACOSX_BUNDLE_BUNDLE_VERSION     ${FPP_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${FPP_VERSION}
        MACOSX_BUNDLE_GUI_IDENTIFIER     "io.github.abhyudayfr.FlowPlusPlus"
    )
endif()

# ── Install ───────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS FlowPlusPlus
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ── Deploy script (Windows and macOS only) ────────────────
# Linux uses Flatpak which handles deployment itself
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    qt_generate_deploy_app_script(
        TARGET FlowPlusPlus
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
