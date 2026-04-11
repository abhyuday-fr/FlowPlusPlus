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

MainWindow::~MainWindow() {}

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
    runMenu->addAction(execAct);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolbar = addToolBar("Main");
    toolbar->setMovable(false);

    QAction *runAct = new QAction("▶  Run", this);
    toolbar->addAction(runAct);
    toolbar->addSeparator();

    QAction *startStopAct = new QAction("Start/Stop", this);
    QAction *processAct = new QAction("Process", this);
    QAction *decisionAct = new QAction("Decision", this);
    QAction *ioAct = new QAction("I/O", this);

    toolbar->addAction(startStopAct);
    toolbar->addAction(processAct);
    toolbar->addAction(decisionAct);
    toolbar->addAction(ioAct);

    connect(startStopAct, &QAction::triggered, this, [this]{armPlacement(FlowNode::NodeType::StartStop, "Start/Stop");});
    connect(processAct, &QAction::triggered, this, [this]{armPlacement(FlowNode::NodeType::Process, "Process");});
    connect(decisionAct, &QAction::triggered, this, [this]{armPlacement(FlowNode::NodeType::Decision, "Decision");});
    connect(ioAct, &QAction::triggered, this, [this]{armPlacement(FlowNode::NodeType::IO, "I/O");});
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("FlowPlusPlus ready.");
}

void MainWindow::armPlacement(FlowNode::NodeType type, const QString &label){
    m_scene->setPlacementMode(type);
    m_view->setCursor(Qt::CrossCursor);
    statusBar()->showMessage("Click on canvas to place: " + label);
}
