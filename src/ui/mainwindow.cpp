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
    toolbar->addAction("▶  Run");
    toolbar->addSeparator();
    toolbar->addAction("Start/Stop");
    toolbar->addAction("Process");
    toolbar->addAction("Decision");
    toolbar->addAction("I/O");
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("FlowPlusPlus ready.");
}
