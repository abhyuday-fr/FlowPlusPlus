#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "FlowNode.h"
#include "StartStopNode.h"
#include "IONode.h"

class FlowScene;
class FlowView;

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void fitToContents();

private:
    void setupMenuBar();
    void setupStatusBar();
    void setupToolBar();

    void armPlacement(FlowNode::NodeType type,
                      const QString &label,
                      StartStopNode::Mode ssMode = StartStopNode::Mode::Start,
                      bool ioInput = false);

    void runFlow();

    FlowScene *m_scene;
    FlowView *m_view;

    // pending placement state
    StartStopNode::Mode m_pendingSSMode = StartStopNode::Mode::Start;
    bool m_pendingIOInput = false;
};

#endif // MAINWINDOW_H
