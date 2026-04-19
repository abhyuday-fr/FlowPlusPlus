#include "mainwindow.h"
#include "FlowScene.h"
#include "FlowView.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("FlowPlusPlus");
    resize(1280, 800);

    m_scene = new FlowScene(this);
    m_view  = new FlowView(m_scene, this);
    setCentralWidget(m_view);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();
}

MainWindow::~MainWindow() {
    // clear scene explicitly before Qt destroys children
    // this prevents connection destructors from accessing deleted nodes
    m_scene->clearAll();
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *newAct = new QAction("&New", this);
    newAct->setShortcut(QKeySequence::New);
    fileMenu->addAction(newAct);

    QAction *openAct = new QAction("&Open", this);
    openAct->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAct);

    QAction *saveAct = new QAction("&Save", this);
    saveAct->setShortcut(QKeySequence::Save);
    fileMenu->addAction(saveAct);

    fileMenu->addSeparator();

    QAction *quitAct = new QAction("&Quit", this);
    quitAct->setShortcut(QKeySequence::Quit);
    connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(quitAct);

    QMenu *runMenu = menuBar()->addMenu("&Run");
    QAction *execAct = new QAction("&Execute Flow", this);
    execAct->setShortcut(QKeySequence("F5"));
    connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(execAct);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolbar = addToolBar("Main");
    toolbar->setMovable(false);

    QAction *runAct = new QAction("▶  Run", this);
    toolbar->addAction(runAct);
    toolbar->addSeparator();

    QAction *startAct = new QAction("Start", this);
    QAction *stopAct = new QAction("Stop", this);
    QAction *processAct = new QAction("Process", this);
    QAction *decisionAct = new QAction("Decision", this);
    QAction *inputAct = new QAction("Input", this);
    QAction *outputAct = new QAction("Output", this);

    toolbar->addAction(startAct);
    toolbar->addAction(stopAct);
    toolbar->addAction(processAct);
    toolbar->addAction(decisionAct);
    toolbar->addAction(inputAct);
    toolbar->addAction(outputAct);

    connect(startAct, &QAction::triggered, this, [this]{ armPlacement(FlowNode::NodeType::StartStop, "Start", StartStopNode::Mode::Start); });
    connect(stopAct,  &QAction::triggered, this, [this]{ armPlacement(FlowNode::NodeType::StartStop, "Stop", StartStopNode::Mode::Stop); });
    connect(processAct, &QAction::triggered, this, [this]{armPlacement(FlowNode::NodeType::Process, "Process");});
    connect(decisionAct, &QAction::triggered, this, [this]{armPlacement(FlowNode::NodeType::Decision, "Decision");});
    connect(inputAct,    &QAction::triggered, this, [this]{ armPlacement(FlowNode::NodeType::IO, "Input", StartStopNode::Mode::Start, true); });
    connect(outputAct,   &QAction::triggered, this, [this]{ armPlacement(FlowNode::NodeType::IO, "Output", StartStopNode::Mode::Start, false); });
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("FlowPlusPlus ready.");
}

void MainWindow::armPlacement(FlowNode::NodeType type, const QString &label, StartStopNode::Mode ssMode, bool ioInput){
    m_pendingSSMode  = ssMode;
    m_pendingIOInput = ioInput;
    m_scene->setPlacementMode(type, ssMode, ioInput);
    m_view->setCursor(Qt::CrossCursor);
    statusBar()->showMessage("Click on canvas to place: " + label);
}

void MainWindow::runFlow()
{
    // chapter 7, will be implemented next
    statusBar()->showMessage("Run: coming in Chapter 7!");
}
