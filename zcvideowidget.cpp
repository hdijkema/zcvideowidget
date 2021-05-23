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

zcVideoWidget::zcVideoWidget(QWidget *parent)
  : zcVideoWidget(nullptr, parent)
{
}

zcVideoWidget::zcVideoWidget(Prefs *p, QWidget *parent) : QFrame(parent)
{
    _prefs = p;
    _parent = parent;

    setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
    setAutoFillBackground(true);
    setFrameShape(QFrame::StyledPanel);
    setMouseTracking(true);

    _propagate_events = false;
    _handle_keys = true;

    _player = new QMediaPlayer(this);

    _video_widget = new QVideoWidget(this);
    _player->setVideoOutput(_video_widget);
    _slider = new zcVideoWidgetSlider(Qt::Horizontal, this);
    _time = new QLabel(this);
    _update_slider = true;

    _slider_time = new QLabel(this);
    _slider_time->hide();

    connect(_player, &QMediaPlayer::durationChanged, this, &zcVideoWidget::setDuration);
    connect(_player, &QMediaPlayer::positionChanged, this, &zcVideoWidget::setPosition);
    connect(_player, &QMediaPlayer::mediaStatusChanged, this, &zcVideoWidget::mediaStateChanged);

    connect(_slider, &QSlider::sliderPressed, this, &zcVideoWidget::sliderPressed);
    connect(_slider, &QSlider::sliderMoved, this, &zcVideoWidget::showPositionChange);
    //connect(_slider, &QSlider::valueChanged, this, &zcVideoWidget::seekPosition);
    connect(_slider, &QSlider::sliderReleased, this, &zcVideoWidget::sliderReleased);

    connect(&_timer, &QTimer::timeout, _slider_time, &QLabel::hide);

    _play = new QToolButton();
    _pause = new QToolButton();

    QStyle *st = QApplication::style();

    QAction *play_action = new QAction(st->standardIcon(QStyle::SP_MediaPlay), tr("Afspelen"), this);
    QAction *pause_action = new QAction(st->standardIcon(QStyle::SP_MediaPause), tr("Pauze"), this);

    connect(play_action, &QAction::triggered, this, &zcVideoWidget::play);
    connect(pause_action, &QAction::triggered, this, &zcVideoWidget::pause);

    _play->setDefaultAction(play_action);
    _pause->setDefaultAction(pause_action);

    _mute = new QToolButton();
    QAction *mute_action = new QAction(st->standardIcon(QStyle::SP_MediaVolumeMuted), tr("Geluid uit"), this);
    mute_action->setCheckable(true);
    bool muted = (_prefs) ? _prefs->get("zcVideoWidget.muted", false) : false;
    mute_action->setChecked(muted);
    connect(mute_action, &QAction::toggled, this, &zcVideoWidget::mute);
    _mute->setDefaultAction(mute_action);

    _volume = new zcVideoWidgetSlider(Qt::Horizontal, this);
    _volume->setRange(0, 100);
    _volume->setTickInterval(5);
    _volume->setSingleStep(5);
    int vol = (_prefs) ? _prefs->get("zcVideoWidget.volume", 100) : 100;
    _volume->setValue(vol);
    _player->setVolume(vol);
    connect(_volume, &QSlider::valueChanged, this, &zcVideoWidget::setVolume);

    _fullscreen = new QToolButton();
    QAction *fullscr_action = new QAction(st->standardIcon(QStyle::SP_TitleBarMaxButton), tr("Volledig scherm"), this);
    fullscr_action->setCheckable(true);
    connect(fullscr_action, &QAction::toggled, this, &zcVideoWidget::fullScreen);
    _fullscreen->setDefaultAction(fullscr_action);

    _movie_name = new QLabel(this);
    _close = new QToolButton();
    QAction *close_action = new QAction(st->standardIcon(QStyle::SP_TitleBarCloseButton), tr("Sluiten"), this);
    connect(close_action, &QAction::triggered, this, &zcVideoWidget::hide);
    _close->setDefaultAction(close_action);

    auto addSep = [this](QHBoxLayout *hbox) {
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setLineWidth(1);
        hbox->addWidget(line);
    };

    QHBoxLayout *hbox_title = new QHBoxLayout();
    hbox_title->addWidget(_movie_name, 1);
    hbox_title->addWidget(_close);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(4);
    hbox->addWidget(_play);
    hbox->addWidget(_pause);
    hbox->addWidget(_slider, 4);
    hbox->addWidget(_time);
    addSep(hbox);
    hbox->addWidget(_mute);
    hbox->addWidget(_volume, 1);
    addSep(hbox);
    hbox->addWidget(_fullscreen);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addLayout(hbox_title);
    vbox->addWidget(_video_widget, 1);
    vbox->addLayout(hbox);

    vbox->setMargin(0);
    vbox->setSpacing(0);
    hbox->setMargin(0);
    hbox_title->setMargin(0);

    setLayout(vbox);
}

zcVideoWidget::~zcVideoWidget()
{
    if (_prefs) { delete _prefs; }
}


void zcVideoWidget::lockInput()
{
    _propagate_events = true;
}

void zcVideoWidget::setHandleKeys(bool yes)
{
    _handle_keys = yes;
}

void zcVideoWidget::setVideo(const QUrl &video_url, bool do_play)
{
    _movie_name->setText(video_url.toString());
    _player->setMedia(video_url);
    if (do_play) { play(); }
    else { pause(); }
}

void zcVideoWidget::move(const QPoint &p)
{
    QPoint pp(p);
    if (_parent != nullptr) {
        pp = _parent->mapToGlobal(p);
    }
    QFrame::move(pp);
}

void zcVideoWidget::hideEvent(QHideEvent *)
{
    emit hidden();
}

void zcVideoWidget::play()
{
    _player->play();
}

void zcVideoWidget::pause()
{
    _player->pause();
}

void zcVideoWidget::setDuration(qint64 duration)
{
    QTime tm(QTime::fromMSecsSinceStartOfDay(duration));
    QString tm_s = QString::asprintf("%02d:%02d:%02d", tm.hour(), tm.minute(), tm.second());
    _time->setText(tm_s);

    bool b = _slider->blockSignals(true);
    _slider->setRange(0, duration);
    _slider->setSingleStep(duration / 20);
    _slider->setTickInterval(duration / 20);
    //_slider->setTickPosition(QSlider::TicksBothSides);
    _slider->blockSignals(b);
}

void zcVideoWidget::setPosition(qint64 pos)
{
    if (_update_slider) {
        bool b = _slider->blockSignals(true);
        _slider->setValue(pos);
        _slider->blockSignals(b);

        qint64 t = _slider->maximum() - pos;
        QTime tm(QTime::fromMSecsSinceStartOfDay(t));
        QString tm_s = QString::asprintf("%02d:%02d:%02d", tm.hour(), tm.minute(), tm.second());
        _time->setText(tm_s);
    }
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
    _player->setPosition(pos);
}

void zcVideoWidget::showPositionChange(int pos)
{
    qint64 t = _slider->maximum() - pos;
    QTime tm(QTime::fromMSecsSinceStartOfDay(t));
    QString tm_s = QString::asprintf("%02d:%02d:%02d", tm.hour(), tm.minute(), tm.second());
    _time->setText(tm_s);

    QTime seek_tm(QTime::fromMSecsSinceStartOfDay(pos));
    QString tm_seek = QString::asprintf("%02d:%02d:%02d", seek_tm.hour(), seek_tm.minute(), seek_tm.second());
    _slider_time->setText(tm_seek);

    QRect slider_rect(_slider->pos(), _slider->size());
    QPoint p = mapFromGlobal(QCursor::pos());
    p.setY(slider_rect.center().y() - (_slider_time->height() / 2));
    p.setX(p.x() + 10);

    _slider_time->move(p);
    _slider_time->show();
}

void zcVideoWidget::sliderPressed()
{
    _update_slider = false;
}

void zcVideoWidget::sliderReleased()
{
    _update_slider = true;
    seekPosition(_slider->value());
    _timer.setSingleShot(true);
    _timer.start(500);
}

void zcVideoWidget::mute(bool yes)
{
    if (_prefs) { _prefs->set("zcVideoWidget.muted", yes); }
    _player->setMuted(yes);
}

void zcVideoWidget::setVolume(qint64 v)
{
    if (_prefs) { _prefs->set("zcVideoWidget.volume", static_cast<int>(v)); }
    _player->setVolume(v);
    if (_player->isMuted()) {
        _mute->setChecked(false);
        _player->setMuted(false);
        if (_prefs) { _prefs->set("zcVideoWidget.muted", false); }
    }
}

void zcVideoWidget::fullScreen(bool fscr)
{
    if (fscr) {
        _prev_states = this->windowState();
        this->setWindowState(Qt::WindowFullScreen);
    } else {
        this->setWindowState(_prev_states);
    }
}

void zcVideoWidget::enterEvent(QEvent *event)
{
    if (_propagate_events) {
        releaseMouse();
    }
    QFrame::enterEvent(event);
}

void zcVideoWidget::leaveEvent(QEvent *event)
{
    if (_propagate_events) {
        grabMouse();
    }
    QFrame::leaveEvent(event);
}

void zcVideoWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (_handle_keys) {
        if (event->key() == Qt::Key_Escape) {
            hide();
        }
    }
    QFrame::keyReleaseEvent(event);
}

void zcVideoWidget::mouseReleaseEvent(QMouseEvent *evt)
{
    if (_propagate_events) {
        QPoint p = evt->pos();
        QRect r(rect());

        if (!r.contains(p)) {
            emit clickOutside();
        }
    }

    QFrame::mouseReleaseEvent(evt);
}

void zcVideoWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (_propagate_events) {
        QPoint p = event->pos();
        QRect r(rect());
        if (!r.contains(p)) {
            if (mouseGrabber() != this) { grabMouse(); }
        } else {
            if (mouseGrabber() == this) { releaseMouse(); }
        }
    }

    QFrame::mouseMoveEvent(event);
}




zcVideoWidget::Prefs::~Prefs()
{
}