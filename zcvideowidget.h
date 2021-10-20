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
#include <QMediaPlayer>
#include <QMediaContent>

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

private:
    zcVideoWidgetData   *D;

public:
    // Prefs will be owned and destoyed by this widget
    explicit zcVideoWidget(QWidget *parent = nullptr);
    explicit zcVideoWidget(int flags, QWidget *parent = nullptr);
    explicit zcVideoWidget(Prefs *p, QWidget *parent = nullptr);
    explicit zcVideoWidget(Prefs *p, int flags, QWidget *parent = nullptr);
    ~zcVideoWidget();

public:
    void setVideo(const QUrl &video_url, bool play, const QString &title = "@@URL@@");

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
    void mediaChanged(const QMediaContent &c);
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
    virtual void enterEvent(QEvent *event) override;
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
