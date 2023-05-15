#ifndef MAINDOWNLOADER_H
#define MAINDOWNLOADER_H

#include <QObject>
#include <QWidget>

#include "zcvideowidget.h"

class MainDownloaderData;
class MainDownloader : public QObject, public zcVideoWidget::Downloader
{
    Q_OBJECT
public:
    explicit MainDownloader();
    ~MainDownloader();

private:
    MainDownloaderData *D;

    // Downloader interface
public:
    bool download(QWidget *parent, const QUrl &http_url, const QFile &to_file, zcVideoWidget *vw);
    QDir downloadDir();
};

#endif // MAINDOWNLOADER_H
