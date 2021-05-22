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

#include "zcvideowidgetslider.h"

class ZCVIDEOWIDGET_EXPORT zcVideoWidget : public QFrame
{
    Q_OBJECT
public:
    class Prefs {
    public:
        virtual bool get(const QString &key, bool def) = 0;
        virtual bool get(const QString &key, int def) = 0;
        virtual bool set(const QString &key, bool v) = 0;
        virtual bool set(const QString &key, int v) = 0;
    public:
        virtual ~Prefs();
    };
private:
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

    QTimer           _timer;

    bool             _propagate_events;
    bool             _handle_keys;

    QPoint           _cur_pos;
    QSize            _cur_size;

    QWidget         *_parent;

    bool             _update_slider;

    Qt::WindowStates _prev_states;

    Prefs           *_prefs;

public:
    // Prefs will be owned and destoyed by this widget
    explicit zcVideoWidget(QWidget *parent);
    explicit zcVideoWidget(Prefs *p, QWidget *parent = nullptr);
    ~zcVideoWidget();

public:
    void setVideo(const QUrl &video_url, bool play);

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

    // QWidget interface
protected:
    virtual void mouseReleaseEvent(QMouseEvent *evt) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void hideEvent(QHideEvent *) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
};


#endif // ZCVIDEOWIDGET_H
