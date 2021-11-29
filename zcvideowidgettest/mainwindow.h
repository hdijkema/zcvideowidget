#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindowData;
class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    MainWindowData *D;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_Video_triggered();

    void on_actionQuit_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
