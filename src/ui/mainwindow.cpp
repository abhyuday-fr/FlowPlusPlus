#include "mainwindow.h"
#include "FlowScene.h"
#include "FlowView.h"
#include "Interpreter.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QKeyEvent>

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
    runMenu->addAction(execAct);
    connect(execAct, &QAction::triggered, this, &MainWindow::runFlow);
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

    connect(runAct, &QAction::triggered, this, &MainWindow::runFlow);

    toolbar->addSeparator();

    QAction *zoomInAct  = new QAction("🔍+", this);
    QAction *zoomOutAct = new QAction("🔍-", this);
    QAction *zoomFitAct = new QAction("⊡ Fit", this);

    zoomInAct->setShortcut(QKeySequence("Ctrl++"));
    zoomOutAct->setShortcut(QKeySequence("Ctrl+-"));
    zoomFitAct->setShortcut(QKeySequence("Ctrl+0"));

    zoomInAct->setToolTip("Zoom In (Ctrl++)");
    zoomOutAct->setToolTip("Zoom Out (Ctrl+-)");
    zoomFitAct->setToolTip("Fit to window (Ctrl+0)");

    toolbar->addAction(zoomInAct);
    toolbar->addAction(zoomOutAct);
    toolbar->addAction(zoomFitAct);

    connect(zoomInAct,  &QAction::triggered, m_view, &FlowView::zoomIn);
    connect(zoomOutAct, &QAction::triggered, m_view, &FlowView::zoomOut);
    connect(zoomFitAct, &QAction::triggered, this,   &MainWindow::fitToContents);
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
    Interpreter interp(m_scene);
    QStringList output = interp.run();

    // show output in a popup dialog
    QDialog *dlg = new QDialog(nullptr);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle("FlowPlusPlus Output");
    dlg->resize(480, 320);

    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Monospace", 10));
    textEdit->setStyleSheet(
        "background-color: #1e1e1e; color: #d4d4d4; border: none;");

    if(output.isEmpty())
        textEdit->setPlainText("(no output)");
    else
        textEdit->setPlainText(output.join("\n"));

    QPushButton *closeBtn = new QPushButton("Close", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    layout->addWidget(textEdit);
    layout->addWidget(closeBtn);

    dlg->exec();
}

void MainWindow::fitToContents(){
    QRectF bounds = m_scene->itemsBoundingRect();
    if (bounds.isEmpty()) return;
    bounds.adjust(-40, -40, 40, 40); // padding
    m_view->fitInView(bounds, Qt::KeepAspectRatio);
}
