#include "maindownloader.h"

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>

class MainDownloaderData
{
public:
    bool     _problem;
    QDialog *_dlg;
    QLabel  *_progress;
    QFile    _to_file;
    int      _prev;
    zcVideoWidget *_vw;
    QNetworkReply *_repl;
};

MainDownloader::MainDownloader()
    : QObject(nullptr)
{
    D = new MainDownloaderData();
    D->_problem = false;
}

MainDownloader::~MainDownloader()
{
    if (D) {
        clearCache();
        delete D;
        D = nullptr;
    }
}

void MainDownloader::clearCache()
{
    D->_vw->clearVideo();
    QDir d(downloadDir());
    QStringList files(d.entryList(QDir::NoDotAndDotDot | QDir::Files));
    for(const QString &file : files) {
        QFile f(d.filePath(file));
        f.remove();
    }
    d.removeRecursively();
}

bool MainDownloader::download(QWidget *parent, const QUrl &http_url, const QFile &to_file, zcVideoWidget *vw)
{
    if (D->_problem) { return false; }

    D->_problem = false;
    D->_vw = vw;

    QDir d(downloadDir());

    D->_dlg = new QDialog(parent);
    D->_dlg->setWindowTitle("Downloading video to local cache");
    QGridLayout *l = new QGridLayout();
    l->addWidget(new QLabel("Downloading:", D->_dlg), 0, 0);
    l->addWidget(new QLabel(http_url.toString(), D->_dlg), 0, 1);
    l->addWidget(new QLabel("To:", D->_dlg), 1, 0);
    l->addWidget(new QLabel(to_file.fileName(), D->_dlg), 1, 1);
    l->addWidget(new QLabel("Progress:", D->_dlg), 2, 0);

    D->_progress = new QLabel("0%", D->_dlg);
    l->addWidget(D->_progress, 2, 1);

    D->_dlg->setLayout(l);

    D->_to_file.setFileName(to_file.fileName());
    if (!D->_to_file.open(QIODevice::WriteOnly)) {
        D->_problem = true;
        return false;
    }

    D->_dlg->show();

    QNetworkAccessManager *mgr = new QNetworkAccessManager(D->_dlg);
    QNetworkRequest req(http_url);
    QNetworkReply *repl = mgr->get(req);
    D->_repl = repl;
    D->_prev = -1;
    connect(repl, &QNetworkReply::downloadProgress, D->_dlg, [this](qint64 received, qint64 total) {
        if (total <= 0) { total = 1; }
        qreal perc = (static_cast<qreal>(received) / static_cast<qreal>(total));
        int i_perc = (perc * 10);
        if (i_perc != D->_prev) {
            D->_prev = i_perc;
            D->_progress->setText(QString("%1%2").arg(QString::asprintf("%.1lf", (perc + 1) * 100), QString("%")));
        }
    });

    connect(repl, &QNetworkReply::readyRead, D->_dlg, [this]() {
        D->_to_file.write(D->_repl->readAll());
    });

    connect(repl, &QNetworkReply::finished, D->_dlg, [this]() {
        if (!D->_problem) {
            D->_to_file.write(D->_repl->readAll());
            D->_to_file.close();
            D->_vw->videoDownloaded();
        }
        D->_dlg->close();
        D->_dlg->deleteLater();
    });

    connect(repl, &QNetworkReply::errorOccurred, D->_dlg, [this]() {
        D->_to_file.close();
        D->_to_file.remove();
        D->_problem = true;
        D->_vw->cannotDownloadVideo();
        D->_dlg->close();
        D->_dlg->deleteLater();
    });

    return true;
}

QDir MainDownloader::downloadDir() {
    D->_problem = false;
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (path == "") { D->_problem = true; return QDir(""); }
    else {
        QDir d(path);
        if (!d.exists("video_cache")) {
            if (!d.mkdir("video_cache")) { D->_problem = true; return QDir(""); }
        }
        QDir dd(d.filePath("video_cache"));
        return dd;
    }
}

