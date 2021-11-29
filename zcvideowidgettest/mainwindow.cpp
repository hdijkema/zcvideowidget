#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "zcvideowidget.h"

#include <QFileDialog>
#include <QRegularExpression>

class MainWindowData
{
public:
    zcVideoWidget   *_video_widget;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    D = new MainWindowData();

    QWidget *w = ui->w_video_plane;
    D->_video_widget = new zcVideoWidget(w);
    QVBoxLayout *vbox = new QVBoxLayout();
    w->setLayout(vbox);
    vbox->addWidget(D->_video_widget);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionOpen_Video_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, "Open Video");

    QUrl u(QUrl::fromLocalFile(file));
    D->_video_widget->setVideo(u, true);

    QString p = file;
    QRegularExpression re("[.][^.]+$");
    p.replace(re, ".srt");
    QFile f(p);
    if (f.exists()) {
        D->_video_widget->setSrt(f);
    }
}


void MainWindow::on_actionQuit_triggered()
{
    close();
}

