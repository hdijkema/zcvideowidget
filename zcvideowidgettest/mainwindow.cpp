#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "zcvideowidget.h"
#include "zcvideodock.h"
#include "maindownloader.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QRegularExpression>
#include <QSettings>
#include <QMessageBox>

class MyPref : public zcVideoWidget::Prefs
{
private:
    QSettings *settings;

public:
    MyPref() { settings = new QSettings("Dijkema", "zcVideWidgetTest"); }
   ~MyPref() { delete settings; }

    // Prefs interface
public:
    bool get(const QString &key, bool def) { return settings->value(key, def).toBool(); }
    int get(const QString &key, int def) { return settings->value(key, def).toInt(); }
    void set(const QString &key, bool v) { settings->setValue(key, v); }
    void set(const QString &key, int v) { settings->setValue(key, v); }
};

class MainWindowData
{
public:
    zcVideoWidget   *_video_widget;
    zcVideoDock     *_dock;
    QUrl             _file_url;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    D = new MainWindowData();
    D->_dock = nullptr;
    D->_video_widget = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_Video_triggered()
{
    if (D->_video_widget) {
        QString file = QFileDialog::getOpenFileName(this, "Open Video");

        QUrl u(QUrl::fromLocalFile(file));
        D->_video_widget->setVideo(u, true);
        this->setWindowTitle(u.toString());

        QString p = file;
        QRegularExpression re("[.][^.]+$");
        p.replace(re, ".srt");
        QFile f(p);
        if (f.exists()) {
            D->_video_widget->setSrt(f);
        }
    } else {
        QMessageBox::warning(this, "Add video frame first", "First, add a videoframe");
    }
}

void MainWindow::on_actionQuit_triggered()
{
    D->_video_widget->clearVideo();
    close();
}


void MainWindow::on_actionOpen_Url_triggered()
{
    if (D->_video_widget) {
        QString Url = QInputDialog::getText(this, "Give URL to open", "Url:");

        QUrl u(Url);
        D->_video_widget->setVideo(u, true);
    } else {
        QMessageBox::warning(this, "Add video frame first", "First, add a videoframe");
    }
}


void MainWindow::on_actionDock_Url_triggered()
{
    QString Url = QInputDialog::getText(this, "Give URL to open", "Url:");
    QUrl file_url(Url);
    D->_file_url = file_url;

    D->_dock = new zcVideoDock(new MainDownloader(), new MyPref(),
                            zcVideoFlags::FLAG_KEEP_POSITION_AND_SIZE|
                            zcVideoFlags::FLAG_HIDE_CONTROLS_FULLSCREEN|
                            zcVideoFlags::FLAG_PREVENT_SLEEP_FULLSCREEN
                            ,
                            this
                            );
    zcVideoWidget *_video_widget = D->_dock->videoWidget();
    D->_dock->setObjectName("mmwiki");

    if (D->_dock) {
        D->_dock->setVisible(true);
    }

    _video_widget->setVideo(D->_file_url, true);

    {
        QString f(file_url.toString());
        QRegularExpression re("[.][^.]+$");
        f.replace(re, ".srt");
        if (f.startsWith("file://")) {
            QUrl fu(f);
            f = fu.toLocalFile();
        }
        QFile ff(f);
          if (ff.exists()) {
            _video_widget->setSrt(ff);
        }
    }

    connect(_video_widget, &zcVideoWidget::hidden,
            D->_dock, [this]() {
                    if (D->_dock != nullptr) { D->_dock->deleteLater(); D->_dock = nullptr; }
            }
    );

    connect(D->_dock, &zcVideoDock::destroyed,
            D->_dock, [this]() { D->_dock = nullptr; }
    );
}



void MainWindow::on_actionAdd_Video_Frame_triggered()
{
    if (D->_video_widget) {
        QMessageBox::warning(this, "Already there", "The video frame has already been created");
    } else {
        QWidget *w = ui->w_video_plane;
        D->_video_widget = new zcVideoWidget(new MainDownloader(),
                                             zcVideoFlags::FLAG_HIDE_CONTROLS_FULLSCREEN |
                                             zcVideoFlags::FLAG_NO_FULL_SCREEN_BUTTON,
                                             w
                                             );
        QVBoxLayout *vbox = new QVBoxLayout();
        vbox->setContentsMargins(0, 0, 0, 0);
        w->setLayout(vbox);
        vbox->addWidget(D->_video_widget);
        connect(D->_video_widget, &zcVideoWidget::error, D->_video_widget, [this](int code) {
            QMessageBox::warning(this, "Cannot download video",
                                 QString("%1 cannot be downloaded (code=%2)").arg(D->_video_widget->lastVideoUrl().toString(), QString::number(code))
                                 );
        });
    }
}

