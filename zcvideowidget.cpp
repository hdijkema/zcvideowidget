/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#include "zcvideowidget.h"

#include <QApplication>
#include <QScreen>
#include <QAction>
#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTime>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QToolButton>

#include <QGraphicsVideoItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsDropShadowEffect>
#include <QTextDocument>
#include <QLibrary>

#include <QDebug>

#include "zcvideodock.h"
#include "srtparser.h"
#include "zcvideowidgetslider.h"

#include <QTimer>

#define LINE_DEBUG qDebug() << __FUNCTION__ << __LINE__

static void preventSleep(QWidget *w, bool yes);

struct Srt {
    int     from_ms;
    int     to_ms;
    QString subtitle;
};

class zcVideoWidgetData {
public:
    int                  _flags;
    QMediaPlayer        *_player;
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
    QWidget             *_controls;

    QTimer               _timer;
    QTimer               _delay_timer;
    QTimer               _cursor_timer;
    QTimer               _control_hide_timer;

    bool                 _propagate_events;
    bool                 _handle_keys;

    QPoint               _cur_pos;
    QSize                _cur_size;

    QWidget             *_parent;

    bool                 _update_slider;

    Qt::WindowStates     _prev_states;
    bool                 _is_fullscreen;

    QGraphicsTextItem   *_srt_item;
    QGraphicsTextItem   *_delay_item;
    QGraphicsVideoItem  *_video_item;
    QGraphicsScene      *_scene;
    QGraphicsView       *_view;
    QFont                _srt_font;
    QString              _current_srt_text;
    int                  _srt_delay;

    zcVideoWidget::Prefs *_prefs;
    bool                 _prefs_first;

    QVector<struct Srt> _subtitles;
};

class zcGraphicsView : public QGraphicsView
{
private:
    zcVideoWidget *_parent;
public:
    zcGraphicsView(zcVideoWidget *parent) : QGraphicsView(parent)
    {
        _parent = parent;
        setMouseTracking(true);

        setFrameShape(NoFrame);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setBackgroundBrush(QBrush(Qt::black));
    }

    // QWidget interface
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override
    {
        _parent->mouseAt(mapToParent(event->pos()));
        QGraphicsView::mouseMoveEvent(event);
    }
};


typedef QWidget super;

zcVideoWidget::zcVideoWidget(QWidget *parent)
  : zcVideoWidget(nullptr, parent)
{}

zcVideoWidget::zcVideoWidget(int flags, QWidget *parent)
    : zcVideoWidget(nullptr, flags, parent)
{}

zcVideoWidget::zcVideoWidget(zcVideoWidget::Prefs *p, QWidget *parent)
    : zcVideoWidget(p, 0, parent)
{}

zcVideoWidget::zcVideoWidget(zcVideoWidget::Prefs *p, int flags, QWidget *parent)
    : QWidget(parent)
{
    D = new zcVideoWidgetData();

    D->_flags = flags;
    D->_prefs = p;
    D->_parent = parent;
    D->_prefs_first = true;

    setAutoFillBackground(true);

    D->_propagate_events = false;
    D->_handle_keys = true;

    D->_is_fullscreen = false;

    D->_player = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    D->_player->setNotifyInterval(100);

    D->_video_item = new QGraphicsVideoItem();
    D->_player->setVideoOutput(D->_video_item);

    D->_srt_item = new QGraphicsTextItem();
    QGraphicsDropShadowEffect *e2 = new QGraphicsDropShadowEffect(this);
    e2->setOffset(1,1);
    D->_srt_item->setGraphicsEffect(e2);

    D->_delay_item = new QGraphicsTextItem();
    QGraphicsDropShadowEffect *e3 = new QGraphicsDropShadowEffect(this);
    e3->setOffset(1,1);
    D->_delay_item->setGraphicsEffect(e3);
    D->_delay_item->hide();

    D->_view = new zcGraphicsView(this);
    D->_scene = new QGraphicsScene();
    D->_view->setScene(D->_scene);

    D->_scene->addItem(D->_video_item);
    D->_scene->addItem(D->_srt_item);
    D->_scene->addItem(D->_delay_item);


    D->_slider = new zcVideoWidgetSlider(Qt::Horizontal, this);
    D->_time = new QLabel(this);
    D->_update_slider = true;

    D->_slider_time = new QLabel(this);
    D->_slider_time->hide();

    connect(D->_player, &QMediaPlayer::durationChanged, this, &zcVideoWidget::setDuration);
    connect(D->_player, &QMediaPlayer::positionChanged, this, &zcVideoWidget::setPosition);
    connect(D->_player, &QMediaPlayer::mediaStatusChanged, this, &zcVideoWidget::mediaStateChanged);
    connect(D->_player, &QMediaPlayer::currentMediaChanged, this, &zcVideoWidget::mediaChanged);

    connect(D->_slider, &QSlider::sliderPressed, this, &zcVideoWidget::sliderPressed);
    connect(D->_slider, &QSlider::sliderMoved, this, &zcVideoWidget::showPositionChange);
    connect(D->_slider, &QSlider::valueChanged, this, &zcVideoWidget::seekPosition);
    connect(D->_slider, &QSlider::sliderReleased, this, &zcVideoWidget::sliderReleased);

    connect(&D->_timer, &QTimer::timeout, D->_slider_time, &QLabel::hide);
    connect(&D->_delay_timer, &QTimer::timeout, this, &zcVideoWidget::clearDelayNotification);
    connect(&D->_cursor_timer, &QTimer::timeout, this, &zcVideoWidget::blankCursorOnView);
    connect(&D->_control_hide_timer, &QTimer::timeout, this, &zcVideoWidget::hideControls);

    D->_play = new QToolButton();
    D->_pause = new QToolButton();

    QStyle *st = QApplication::style();

    QAction *play_action = new QAction(st->standardIcon(QStyle::SP_MediaPlay), tr("Afspelen"), this);
    QAction *pause_action = new QAction(st->standardIcon(QStyle::SP_MediaPause), tr("Pauze"), this);

    connect(play_action, &QAction::triggered, this, &zcVideoWidget::play);
    connect(pause_action, &QAction::triggered, this, &zcVideoWidget::pause);

    D->_play->setDefaultAction(play_action);
    D->_pause->setDefaultAction(pause_action);

    D->_mute = new QToolButton();
    QAction *mute_action = new QAction(st->standardIcon(QStyle::SP_MediaVolumeMuted), tr("Geluid uit"), this);
    mute_action->setCheckable(true);

    connect(mute_action, &QAction::toggled, this, &zcVideoWidget::mute);
    D->_mute->setDefaultAction(mute_action);

    D->_volume = new zcVideoWidgetSlider(Qt::Horizontal, this);
    D->_volume->setTracking(true);
    D->_volume->setRange(0, 100);
    D->_volume->setTickInterval(5);
    D->_volume->setSingleStep(5);
    D->_volume->setValue(0);
    D->_player->setVolume(0);
    connect(D->_volume, &QSlider::valueChanged, this, &zcVideoWidget::setVolume);

    D->_srt_delay = 0;

    D->_fullscreen = new QToolButton();
    QAction *fullscr_action = new QAction(st->standardIcon(QStyle::SP_TitleBarMaxButton), tr("Volledig scherm"), this);
    fullscr_action->setCheckable(true);
    connect(fullscr_action, &QAction::toggled, this, &zcVideoWidget::fullScreen);
    D->_fullscreen->setDefaultAction(fullscr_action);

    auto addSep = [this](QHBoxLayout *hbox) {
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setLineWidth(1);
        hbox->addWidget(line);
    };

    QHBoxLayout *hbox_title = nullptr;
    if (D->_flags&zcVideoFlags::FLAG_SOFT_TITLE) {
        D->_movie_name = new QLabel(this);
        D->_close = new QToolButton();
        QAction *close_action = new QAction(st->standardIcon(QStyle::SP_TitleBarCloseButton), tr("Sluiten"), this);
        connect(close_action, &QAction::triggered, this, &zcVideoWidget::hide);
        D->_close->setDefaultAction(close_action);

        hbox_title = new QHBoxLayout();
        hbox_title->addWidget(D->_movie_name, 1);
        hbox_title->addWidget(D->_close);
    } else {
        D->_movie_name = nullptr;
        D->_close = nullptr;
    }

    D->_controls = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(4);
    hbox->addWidget(D->_play);
    hbox->addWidget(D->_pause);
    hbox->addWidget(D->_slider, 4);
    hbox->addWidget(D->_time);
    addSep(hbox);
    hbox->addWidget(D->_mute);
    hbox->addWidget(D->_volume, 1);
    addSep(hbox);
    hbox->addWidget(D->_fullscreen);
    D->_controls->setLayout(hbox);

    setMouseTracking(true);
    D->_view->setMouseTracking(true);

    QVBoxLayout *vbox = new QVBoxLayout();
    if (D->_flags&zcVideoFlags::FLAG_SOFT_TITLE && hbox_title != nullptr) {
        vbox->addLayout(hbox_title);
    }
    vbox->addWidget(D->_view, 1);
    vbox->addWidget(D->_controls);

    vbox->setMargin(0);
    vbox->setSpacing(0);
    hbox->setMargin(0);
    if (D->_flags&zcVideoFlags::FLAG_SOFT_TITLE && hbox_title != nullptr) {
        hbox_title->setMargin(0);
    }

    setLayout(vbox);
}

zcVideoWidget::~zcVideoWidget()
{
    if (D->_prefs) { delete D->_prefs; }
    delete D;
}


void zcVideoWidget::lockInput()
{
    D->_propagate_events = true;
}

void zcVideoWidget::setHandleKeys(bool yes)
{
    D->_handle_keys = yes;
}

void zcVideoWidget::setVideo(const QUrl &video_url, bool do_play, const QString &_title)
{
    QString title;
    if (_title == "@@URL@@") { title = video_url.toString(); }
    else { title = _title; }

    if (D->_flags&zcVideoFlags::FLAG_SOFT_TITLE) {
        D->_movie_name->setText(title);
    } else {
        if (D->_flags&zcVideoFlags::FLAG_DOCKED) {
            zcVideoDock *w = qobject_cast<zcVideoDock *>(parent());
            if (w != nullptr) {
                w->setWindowTitle(title);
            }
        }
    }
    D->_player->setMedia(video_url);
    if (do_play) { play(); }
    else { pause(); }

    D->_current_srt_text = "";
}

bool zcVideoWidget::setSrt(const QFile &file)
{
    clearSrt();

    if (file.exists()) {
        SubtitleParserFactory *subParserFactory = new SubtitleParserFactory(file.fileName().toStdString());
        SubtitleParser *parser = subParserFactory->getParser();
        std::vector<SubtitleItem*> sub = parser->getSubtitles();
        foreach(SubtitleItem *item, sub) {
            struct Srt srt;
            srt.from_ms = item->getStartTime();
            srt.to_ms = item->getEndTime();
            srt.subtitle = QString::fromStdString(item->getText()).trimmed();
            D->_subtitles.append(srt);
        }
        delete parser;
        delete subParserFactory;
        return true;
    } else {
        qWarning() << __FUNCTION__ << __LINE__ << file.fileName() << " does not exist";
        return false;
    }
}

void zcVideoWidget::clearSrt()
{
    setSrtText("");
    D->_subtitles.clear();
}

void zcVideoWidget::setSrtText(const QString &html_text)
{
    QString objn = objectName();
    if (objn == "") { objn = "default"; }
    QString name = QString("zcVideoWidget.%1").arg(objn);

    QTextOption option = D->_srt_item->document()->defaultTextOption();
    option.setAlignment(Qt::AlignCenter);
    D->_srt_item->document()->setDefaultTextOption(option);

    int fontsize_pt = 12;
    if (D->_prefs) {
        fontsize_pt = D->_prefs->get(QString("%1.fontsize").arg(name), 20);
    }

    D->_srt_item->setDefaultTextColor(Qt::white);
    QString txt = QString("<span style=\"font-size: %1pt;\">%2</span>").arg(QString::number(fontsize_pt), html_text);
    D->_srt_item->setHtml(txt);

    D->_current_srt_text = html_text;
}

void zcVideoWidget::subtitleEarlier(int by_ms)
{
    D->_srt_delay -= by_ms;
    notifySubtitleDelay();
}

void zcVideoWidget::subtitleLater(int by_ms)
{
    D->_srt_delay += by_ms;
    notifySubtitleDelay();
}

void zcVideoWidget::subtitleOnTime()
{
    D->_srt_delay = 0;
    notifySubtitleDelay();
}

void zcVideoWidget::move(const QPoint &p)
{
    QPoint pp(p);
    if (D->_parent != nullptr) {
        pp = D->_parent->mapToGlobal(p);
    }
    super::move(pp);
}

void zcVideoWidget::hideEvent(QHideEvent *)
{
    if (D->_flags&zcVideoFlags::FLAG_PREVENT_SLEEP_FULLSCREEN) {
        preventSleep(this, false);
    }
    emit hidden();
}

void zcVideoWidget::play()
{
    D->_player->play();
}

void zcVideoWidget::pause()
{
    D->_player->pause();
}

void zcVideoWidget::setDuration(qint64 duration)
{
    QTime tm(QTime::fromMSecsSinceStartOfDay(duration));
    QString tm_s = QString::asprintf("%02d:%02d:%02d", tm.hour(), tm.minute(), tm.second());
    D->_time->setText(tm_s);

    bool b = D->_slider->blockSignals(true);
    D->_slider->setRange(0, duration);
    D->_slider->setSingleStep(duration / 20);
    D->_slider->setTickInterval(duration / 20);
    D->_slider->blockSignals(b);
}

void zcVideoWidget::processSrt(int pos_in_ms)
{
    pos_in_ms += (-1 * D->_srt_delay);

    auto in_srt = [this](int ms) {
        auto lower = std::lower_bound(D->_subtitles.begin(), D->_subtitles.end(), ms, [](const struct Srt &srt, int ms) {
            return srt.to_ms < ms;
        });
        if (lower != D->_subtitles.end()) {
            return *lower;
        } else {
            struct Srt s { -1, -1, "" };
            return s;
        }
    };

    auto is_valid_srt = [](const struct Srt &srt) {
        return srt.from_ms >= 0 && srt.to_ms >= 0;
    };

    struct Srt s = in_srt(pos_in_ms);

    if (!is_valid_srt(s) && D->_current_srt_text != "") {
        setSrtText("");
    } if (is_valid_srt(s)) {
        if (pos_in_ms <= s.to_ms && pos_in_ms >= s.from_ms) {
            if (D->_current_srt_text != s.subtitle) {
                adjustSize();
                setSrtText(s.subtitle);
            }
        } else if (D->_current_srt_text != "") {
            setSrtText("");
        }
    }
}

void zcVideoWidget::setPosition(qint64 pos)
{
    if (D->_update_slider) {
        bool b = D->_slider->blockSignals(true);
        D->_slider->setValue(pos);
        D->_slider->blockSignals(b);

        qint64 t = D->_slider->maximum() - pos;
        QTime tm(QTime::fromMSecsSinceStartOfDay(t));
        QString tm_s = QString::asprintf("%02d:%02d:%02d", tm.hour(), tm.minute(), tm.second());
        D->_time->setText(tm_s);
    }

    processSrt(pos);
}

void zcVideoWidget::mediaChanged(const QMediaContent &)
{
}

void zcVideoWidget::mediaStateChanged(QMediaPlayer::MediaStatus st)
{
#ifdef Q_OS_WIN
    QString os = "Windows";
    QString codecs = tr("U kunt eventueel extra video-codecs installeren (KLite Codecs) via deze <a href=\"https://www.codecguide.com/download_kl.htm\">link</a>.");
#endif
#ifdef Q_OS_LINUX
    QString os = "Linux";
    QString codecs = tr("U kunt extra gstreamer video-codecs installeren. Raadpleeg hiervoor de documentatie van uw Linux versie.");
#endif
#ifdef Q_OS_MAC
    QString os = "MacOS";
    QString codecs = tr("Op de mac wordt alleen .mov, .mp4 en .m4v formaat ondersteund. Dit zijn de ondersteunde formaten door QuickTime.");
#endif

    if (st == QMediaPlayer::InvalidMedia) {
        QMessageBox::warning(this,
                                  tr("Video kan niet worden afgespeeld"),
                                  tr("De video kan niet worden afgespeeld op uw machine, "
                                     "omdat het videoformaat niet wordt ondersteund door %1.<br>"
                                     "<br>"
                                     "%2").arg(os, codecs)
                                  );
    }
}

void zcVideoWidget::seekPosition(int pos)
{
    D->_player->setPosition(pos);
}

void zcVideoWidget::showPositionChange(int pos)
{
    qint64 t = D->_slider->maximum() - pos;
    QTime tm(QTime::fromMSecsSinceStartOfDay(t));
    QString tm_s = QString::asprintf("%02d:%02d:%02d", tm.hour(), tm.minute(), tm.second());
    D->_time->setText(tm_s);

    QTime seek_tm(QTime::fromMSecsSinceStartOfDay(pos));
    QString tm_seek = QString::asprintf("%02d:%02d:%02d", seek_tm.hour(), seek_tm.minute(), seek_tm.second());
    D->_slider_time->setText(tm_seek);

    QRect slider_rect(D->_slider->pos(), D->_slider->size());
    QPoint p = mapFromGlobal(QCursor::pos());
    p.setY(slider_rect.center().y() - (D->_slider_time->height() / 2));

    int stw = D->_slider_time->width();

    if (p.x() > (slider_rect.right() - stw)) {
        p.setX(p.x() - stw);
    } else {
        p.setX(p.x() + 10);
    }

    D->_slider_time->move(p);
    D->_slider_time->show();
}

void zcVideoWidget::sliderPressed()
{
    D->_update_slider = false;
}

void zcVideoWidget::sliderReleased()
{
    D->_update_slider = true;
    D->_timer.setSingleShot(true);
    D->_timer.start(500);
}

void zcVideoWidget::mute(bool yes)
{
    QString myname = (objectName() == "") ? "default" : objectName();
    if (D->_prefs) { D->_prefs->set(QString("zcVideoWidget.%1.muted").arg(myname), yes); }
    D->_player->setMuted(yes);
}

void zcVideoWidget::setVolume(qint64 v)
{
    QString myname = (objectName() == "") ? "default" : objectName();
    if (D->_prefs) { D->_prefs->set(QString("zcVideoWidget.%1.volume").arg(myname), static_cast<int>(v)); }
    D->_player->setVolume(v);
    if (D->_player->isMuted()) {
        D->_mute->setChecked(false);
        D->_player->setMuted(false);
        if (D->_prefs) { D->_prefs->set(QString("zcVideoWidget.%1.muted").arg(myname), false); }
    }
}

#ifdef Q_OS_WIN
static uint (*SetThreadExecutionState)(uint esFlags) = nullptr;
const uint ES_CONTINUOUS = 0x80000000;
const uint ES_SYSTEM_REQUIRED = 0x00000001;
const uint ES_DISPLAY_REQUIRED = 0x00000002;
#endif

static void setFunction(void **f, void *g)
{
    *f = g;
}

static void preventSleep(QWidget *w, bool yes)
{
    LINE_DEBUG << w << yes << w->property("zcvideowidget.preventsleep");

#ifdef Q_OS_WIN
    if (SetThreadExecutionState == nullptr) {
        QLibrary lib("kernel32.dll");
        setFunction(reinterpret_cast<void **>(&SetThreadExecutionState), reinterpret_cast<void *>(lib.resolve("SetThreadExecutionState")));
    }

    if (yes) {
        uint previous_state = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
        w->setProperty("zcvideowidget.preventsleep", previous_state);
        if (previous_state == 0) {
            qWarning() << __FUNCTION__ << __LINE__ << "SetThreadExecutionState failed.";
        }
    } else {
        QVariant prev_st = w->property("zcvideowidget.preventsleep");
        if (prev_st.isValid()) {
            uint previous_state = w->property("zcvideowidget.preventsleep").toUInt();
            SetThreadExecutionState(previous_state);
            w->setProperty("zcvideowidget.preventsleep", QVariant());
        }
    }
#endif
}

void zcVideoWidget::doFullScreen(QWidget *w, bool fscr)
{
    bool mainwin = false;
#ifdef Q_OS_WIN32
    QDockWidget *dw = qobject_cast<QDockWidget *>(w);
    QMainWindow *mw = qobject_cast<QMainWindow *>(w);
    mainwin = (mw != nullptr) || (dw != nullptr);
#else
    QMainWindow *mw = qobject_cast<QMainWindow *>(w);
    mainwin = (mw != nullptr);
#endif
    if (mainwin) {
        if (fscr) {
            D->_prev_states = w->windowState();
            w->setWindowState(Qt::WindowFullScreen);
        } else {
            w->setWindowState(D->_prev_states);
        }
    } else {
        if (fscr) {
            QScreen *scr = QApplication::primaryScreen();
            QSize s(w->size());
            QPoint p(w->pos());
            w->setProperty("zcvideowidget.prevsize", s);
            w->setProperty("zcvideowidget.prevpos", p);
            w->move(0, 0);
            w->resize(scr->size());
        } else {
            QSize s(w->property("zcvideowidget.prevsize").toSize());
            QPoint p(w->property("zcvideowidget.prevpos").toPoint());
            w->move(p);
            w->resize(s);
        }
    }

    if (D->_flags&zcVideoFlags::FLAG_PREVENT_SLEEP_FULLSCREEN) {
        preventSleep(this, fscr);
    }
}

#define ABS(a)  ((a < 0) ? -a : a)

void zcVideoWidget::notifySubtitleDelay()
{
    QString text;
    QString delay = QString::number(ABS(D->_srt_delay));
    if (D->_srt_delay < 0) {
        text = QString(tr("Ondertitels %1ms vroeger").arg(delay));
    } else if (D->_srt_delay == 0) {
        text = QString(tr("Ondertitels precies op tijd"));
    } else {
        text = QString(tr("Ondertitels %1ms later").arg(delay));
    }

    QString objn = objectName();
    if (objn == "") { objn = "default"; }
    QString name = QString("zcVideoWidget.%1").arg(objn);

    QTextOption option = D->_srt_item->document()->defaultTextOption();
    option.setAlignment(Qt::AlignLeft);
    D->_delay_item->document()->setDefaultTextOption(option);

    int fontsize_pt = 12;
    if (D->_prefs) {
        fontsize_pt = D->_prefs->get(QString("%1.fontsize").arg(name), 20);
    }

    D->_delay_item->setDefaultTextColor(Qt::white);
    QString txt = QString("<span style=\"font-size: %1pt;\">%2</span>").arg(QString::number(fontsize_pt), text);
    D->_delay_item->setHtml(txt);

    D->_delay_item->setPos(QPoint(5, 5));
    D->_delay_item->show();

    D->_delay_timer.setSingleShot(true);
    D->_delay_timer.start(1000);
}


void zcVideoWidget::clearDelayNotification()
{
    D->_delay_item->hide();
}


void zcVideoWidget::fullScreen(bool fscr)
{
    if (fscr) {
        if (D->_flags&zcVideoFlags::FLAG_DOCKED) {
            zcVideoDock *w = qobject_cast<zcVideoDock *>(parent());
            if (w) {
                doFullScreen(w, fscr);
            }
        } else {
            doFullScreen(this, fscr);
        }
        D->_is_fullscreen = true;
        if (D->_flags&zcVideoFlags::FLAG_HIDE_CONTROLS_FULLSCREEN) {
            D->_control_hide_timer.setSingleShot(true);
            D->_control_hide_timer.start(3000);
        }
    } else {
        if (D->_flags&zcVideoFlags::FLAG_DOCKED) {
            zcVideoDock *w = qobject_cast<zcVideoDock *>(parent());
            if (w) {
                doFullScreen(w, fscr);
            }
        } else {
            doFullScreen(this, fscr);
        }
        D->_is_fullscreen = false;
        D->_cursor_timer.stop();
        if (D->_flags&zcVideoFlags::FLAG_HIDE_CONTROLS_FULLSCREEN) {
            D->_control_hide_timer.stop();
            showControls();
        }
    }

    adjustSize();
}

void zcVideoWidget::enterEvent(QEvent *event)
{
    if (D->_propagate_events) {
        releaseMouse();
    }
    super::enterEvent(event);
}

void zcVideoWidget::leaveEvent(QEvent *event)
{
    if (D->_propagate_events) {
        grabMouse();
    }
    super::leaveEvent(event);
}

void zcVideoWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (D->_handle_keys) {
        if (event->key() == Qt::Key_Escape) {
            hide();
        } else if (event->key() == Qt::Key_Z) {
            subtitleEarlier(100);
        } else if (event->key() == Qt::Key_X) {
            subtitleLater(100);
        }
    }
    super::keyReleaseEvent(event);
}

void zcVideoWidget::closeEvent(QCloseEvent *event)
{
    hide();
    super::closeEvent(event);
}

void zcVideoWidget::adjustSize()
{
    QRect r(D->_view->viewport()->rect());
    D->_scene->setSceneRect(r);
    D->_video_item->setSize(r.size());

    setSrtText(D->_current_srt_text);

    int width = r.size().width();
    D->_srt_item->setTextWidth(width);

    int height = r.size().height();
    int bheight = D->_srt_item->boundingRect().height();

    int h = height - bheight * 2;
    int hh = height - (0.1 * height);
    if (h > hh) { h = hh; }
    if ((height - bheight) < h) { h = height - (bheight * 1.25); }

    D->_srt_item->setPos(0, h);
}

void zcVideoWidget::showEvent(QShowEvent *event)
{
    if (D->_prefs_first) {
        D->_prefs_first = false;

        QString myname = (objectName() == "") ? "default" : objectName();

        bool muted = (D->_prefs) ? D->_prefs->get(QString("zcVideoWidget.%1.muted").arg(myname), false) : false;

        auto mute_action = D->_mute->actions().first();
        bool b = mute_action->blockSignals(true);
        mute_action->setChecked(muted);
        mute_action->blockSignals(b);

        int vol = (D->_prefs) ? D->_prefs->get(QString("zcVideoWidget.%1.volume").arg(myname), 100) : 100;
        b= D->_volume->blockSignals(true);
        D->_volume->setValue(vol);
        D->_volume->blockSignals(b);

        D->_player->setVolume(vol);
    }

    adjustSize();
    super::showEvent(event);
}

void zcVideoWidget::resizeEvent(QResizeEvent *event)
{
    super::resizeEvent(event);
    adjustSize();
}

void zcVideoWidget::mouseReleaseEvent(QMouseEvent *evt)
{
    if (D->_propagate_events) {
        QPoint p = evt->pos();
        QRect r(rect());

        if (!r.contains(p)) {
            emit clickOutside();
        }
    }

    super::mouseReleaseEvent(evt);
}

void zcVideoWidget::mouseAt(QPoint p)
{
    QRect r(rect());

    if (D->_propagate_events) {
        if (!r.contains(p)) {
            if (mouseGrabber() != this) { grabMouse(); }
        } else {
            if (mouseGrabber() == this) { releaseMouse(); }
        }
    }

    QRect rr(D->_view->rect());

    setCursor(Qt::ArrowCursor);
    if (D->_is_fullscreen) {
        if (rr.contains(p)) {
            D->_cursor_timer.setSingleShot(true);
            D->_cursor_timer.start(3000);
        } else {
            D->_cursor_timer.stop();
        }
        if (D->_flags&zcVideoFlags::FLAG_HIDE_CONTROLS_FULLSCREEN) {
            D->_control_hide_timer.setSingleShot(true);
            D->_control_hide_timer.start(3000);
            showControls();
        }
    } else {
        if (D->_flags&zcVideoFlags::FLAG_HIDE_CONTROLS_FULLSCREEN) {
            showControls();
        }
    }
}

void zcVideoWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    mouseAt(p);
    super::mouseMoveEvent(event);
}

void zcVideoWidget::blankCursorOnView()
{
    setCursor(Qt::BlankCursor);
}

void zcVideoWidget::hideControls()
{
    if (D->_controls->isVisible()) {
        D->_controls->setVisible(false);
    }
}

void zcVideoWidget::showControls()
{
    if (!D->_controls->isVisible()) {
        D->_controls->setVisible(true);
    }
}
