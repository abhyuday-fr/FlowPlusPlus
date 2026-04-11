#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FlowScene;
class FlowView;

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupMenuBar();
    void setupStatusBar();
    void setupToolBar();

    FlowScene *m_scene;
    FlowView *m_view;
};

#endif // MAINWINDOW_H
