/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#ifndef ZCVIDEOWIDGET_H
#define ZCVIDEOWIDGET_H

#include "zcvideowidget_global.h"
#include "zcvideoflags.h"
#include <QWidget>
#include <QFile>
#include <QDir>

#ifdef QT6
#include <QMediaPlayer>
#else
#include <QMediaPlayer>
#include <QMediaContent>
#endif

class zcVideoWidgetData;
class zcGraphicsView;

class ZCVIDEOWIDGET_EXPORT zcVideoWidget : public QWidget
{
    Q_OBJECT
public:
    class Prefs {
    public:
        virtual bool get(const QString &key, bool def) = 0;
        virtual int  get(const QString &key, int def) = 0;
        virtual void set(const QString &key, bool v) = 0;
        virtual void set(const QString &key, int v) = 0;
    public:
        virtual ~Prefs() {}
    };

    /**
     * Beware! If you create a subclass with a QObject for this entry, make sure, you don't
     * make it autodelete with the parent widget. The zcVideoWidget will be it's parent and
     * will explicitly delete the object.
     */
    class Downloader {
    public:
        /* Will download the 'http_url' to 'to_file' and afterwards call vw->videoDownloaded(),
         * which will start a local play of the given url. This is to overcome the shortcome with MacOS,
         * that will not play streaming video (Qt 6.5.0). Cleaning up after playing the video, is left
         * to the programmer using this widget. the downloader can also download .srt files, which
         * can be set wit vw->setSrt().
         *
         * If the video cannot be downloaded, zcVideoWidget should be informed about it using
         * vw->cannotDownloadVideo()
         *
         * returnvalue: false, if download cannot be executed.
         *
         * zcVideoWidget will emit an error (signal error(int code)) when the video cannot be downloaded,
         * or should be downloaded but there's no downloader. Also calling cannotDownloadVideo() will
         * trigger the emit.
         *
         */
        virtual bool download(QWidget *parent, const QUrl &http_url, const QFile &to_file, zcVideoWidget *vw) = 0;

        /* This function must return a directory. It will be used to make a local file 'to_file' for the download function */
        virtual QDir downloadDir() = 0;
    public:
        virtual ~Downloader() {};
    };

private:
    zcVideoWidgetData   *D;

public:
    static const int ERR_NO_DOWNLOADER = 1;
    static const int ERR_CANNOT_DOWNLOAD = 2;

public:
    // Prefs will be owned and destoyed by this widget
    explicit zcVideoWidget(QWidget *parent = nullptr);
    explicit zcVideoWidget(int flags, QWidget *parent = nullptr);
    explicit zcVideoWidget(Prefs *p, QWidget *parent = nullptr);
    explicit zcVideoWidget(Prefs *p, int flags, QWidget *parent = nullptr);
    explicit zcVideoWidget(Downloader *d, QWidget *parent = nullptr);
    explicit zcVideoWidget(Downloader *d, int flags, QWidget *parent = nullptr);
    explicit zcVideoWidget(Downloader *d, Prefs *p, int flags, QWidget *parent = nullptr);
    ~zcVideoWidget();

public:
    void setVideo(const QUrl &video_url, bool play, const QString &title = "@@URL@@");
    void clearVideo();
    QUrl lastVideoUrl();
    void setTitle(const QString &title);

public:
    void videoDownloaded();
    void cannotDownloadVideo();

public:
    bool setSrt(const QFile &file);
    void clearSrt();
    void setSrtText(const QString &html_text);

    void subtitleEarlier(int by_ms);
    void subtitleLater(int by_ms);
    void subtitleOnTime();

public:
    void move(const QPoint &p);

public:
    void lockInput();
    void setHandleKeys(bool yes);

public:
    void handleDockChange(bool scr);

signals:
    void hidden();
    void clickOutside();
    void signalSetVideo();
    void error(int code);

public slots:
    void play();
    void pause();

private slots:
    void setDuration(qint64 duration);
    void setPosition(qint64 pos);
    void seekPosition(int pos);
    void showPositionChange(int pos);
    void sliderPressed();
    void sliderReleased();
    void mute(bool yes);
    void setVolume(qint64 v);
    void fullScreen(bool yes);
    void mediaStateChanged(QMediaPlayer::MediaStatus st);
    void execSetVideo();

#ifdef QT6
    void mediaChanged(const QUrl &media);
    void newAudioOutput();
#else
    void mediaChanged(const QMediaContent &c);
#endif
    void clearDelayNotification();
    void blankCursorOnView();
    void hideControls();
    void showControls();

private:
    void fullScreenAct(bool yes, bool act);

    // QWidget interface
protected:
    virtual void mouseReleaseEvent(QMouseEvent *evt) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void hideEvent(QHideEvent *) override;
#ifdef QT6
    virtual void enterEvent(QEnterEvent *event) override;
#else
    virtual void enterEvent(QEvent *event) override;
#endif
    virtual void leaveEvent(QEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    void adjustSize();
    void processSrt(int pos_in_ms);
    void doFullScreen(QWidget *w, bool fscr);
    void notifySubtitleDelay();
    void mouseAt(QPoint p);

    friend zcGraphicsView;

};


#endif // ZCVIDEOWIDGET_H
