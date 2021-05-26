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

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QSlider>
#include <QToolButton>
#include <QFrame>
#include <QLabel>
#include <QTimer>
#include <QFile>
#include <QVector>
#include <QGraphicsVideoItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>

#include "zcvideowidgetslider.h"
#include "zcvideoflags.h"

class ZCVIDEOWIDGET_EXPORT zcVideoWidget : public QWidget
{
    Q_OBJECT
private:
    struct Srt {
        int     from_ms;
        int     to_ms;
        QString subtitle;
    };

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
    int                  _flags;
    QMediaPlayer        *_player;
    QVideoWidget        *_video_widget;
    zcVideoWidgetSlider *_slider;
    QToolButton         *_play;
    QToolButton         *_pause;
    QLabel              *_time;
    QLabel              *_slider_time;
    zcVideoWidgetSlider *_volume;
    QToolButton         *_mute;
    QToolButton         *_fullscreen;
    QLabel              *_movie_name;
    QToolButton         *_close;

    QTimer               _timer;

    bool                 _propagate_events;
    bool                 _handle_keys;

    QPoint               _cur_pos;
    QSize                _cur_size;

    QWidget             *_parent;

    bool                 _update_slider;

    Qt::WindowStates     _prev_states;

    QGraphicsTextItem   *_srt_item;
    QGraphicsVideoItem  *_video_item;
    QGraphicsScene      *_scene;
    QGraphicsView       *_view;
    QFont                _srt_font;
    QString              _current_srt_text;

    Prefs               *_prefs;
    bool                 _prefs_first;

    QVector<struct Srt> _subtitles;

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

public:
    void move(const QPoint &p);

public:
    void lockInput();
    void setHandleKeys(bool yes);

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
};


#endif // ZCVIDEOWIDGET_H
