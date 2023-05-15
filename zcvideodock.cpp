/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#include "zcvideodock.h"
#include <QEvent>

typedef QMainWindow     super;

class zcVideoDockData
{
public:
    zcVideoWidget           *_video_widget;
    zcVideoWidget::Prefs    *_prefs;            // will be owned and deleted by zcVideoWidget
    int                      _flags;
};

zcVideoDock::zcVideoDock(QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(nullptr, nullptr, 0, parent, wflags)
{
}

zcVideoDock::zcVideoDock(zcVideoWidget::Downloader *d, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(d, nullptr, 0, parent, wflags)
{}

zcVideoDock::zcVideoDock(zcVideoWidget::Prefs *p, int flags, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(nullptr, p, flags, parent, wflags)
{}

zcVideoDock::zcVideoDock(zcVideoWidget::Prefs *p, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(nullptr, p, 0, parent, wflags)
{}

zcVideoDock::zcVideoDock(int flags, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(nullptr, nullptr, flags, parent, wflags)
{}

zcVideoDock::zcVideoDock(zcVideoWidget::Downloader *d, int flags, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(d, nullptr, flags, parent, wflags)
{}

zcVideoDock::zcVideoDock(zcVideoWidget::Downloader *d, zcVideoWidget::Prefs *p, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(d, p, 0, parent, wflags)
{}

zcVideoDock::zcVideoDock(zcVideoWidget::Downloader *d, zcVideoWidget::Prefs *p, int flags, QWidget *parent, Qt::WindowFlags wflags)
    : QMainWindow(parent, wflags)
{
    D = new zcVideoDockData();

    D->_flags = flags;
    D->_prefs = p;
    D->_video_widget = new zcVideoWidget(d, p, flags|zcVideoFlags::FLAG_DOCKED, this);

    setCentralWidget(D->_video_widget);
}


zcVideoDock::~zcVideoDock()
{
    delete D->_video_widget;
    delete D;
}


bool zcVideoDock::hasPositionAndSize()
{
    zcVideoWidget::Prefs *_prefs = D->_prefs;
    if (_prefs == nullptr) {
        return false;
    } else {
        QString objn = objectName();
        if (objn == "") { objn = "default"; }
        QString name = QString("zcVideoDock.%1").arg(objn);

        int x = _prefs->get(QString("%1.x").arg(name), -1);
        int y = _prefs->get(QString("%1.y").arg(name), -1);
        int w = _prefs->get(QString("%1.w").arg(name), -1);
        int h = _prefs->get(QString("%1.h").arg(name), -1);

        return x >= 0 && y >= 0 && w >=0 && h >=0;
    }
}

zcVideoWidget *zcVideoDock::videoWidget()
{
    return D->_video_widget;
}

void zcVideoDock::setObjectName(const QString &name)
{
    super::setObjectName(name);
    D->_video_widget->setObjectName(name);
}

void zcVideoDock::showEvent(QShowEvent *event)
{
    QString objn = objectName();
    if (objn == "") { objn = "default"; }
    QString name = QString("zcVideoDock.%1").arg(objn);
    zcVideoWidget::Prefs *_prefs = D->_prefs;
    int _flags = D->_flags;

    if (_prefs && _flags&zcVideoFlags::FLAG_KEEP_POSITION_AND_SIZE) {
        int x = _prefs->get(QString("%1.x").arg(name), -1);
        int y = _prefs->get(QString("%1.y").arg(name), -1);
        int w = _prefs->get(QString("%1.w").arg(name), -1);
        int h = _prefs->get(QString("%1.h").arg(name), -1);
        if (x >= 0 && y >= 0 && w >= 0 && h >= 0) {
            if (w < 50) { w = 50; }
            if (h < 50) { h = 50; }
            QPoint p(x, y);
            QSize s(w, h);
            move(p);
            resize(s);
        }
    }

    super::showEvent(event);
}

void zcVideoDock::hideEvent(QHideEvent *event)
{
    QString objn = objectName();
    if (objn == "") { objn = "default"; }
    QString name = QString("zcVideoDock.%1").arg(objn);

    zcVideoWidget::Prefs *_prefs = D->_prefs;
    int _flags = D->_flags;

    if (_prefs && _flags&zcVideoFlags::FLAG_KEEP_POSITION_AND_SIZE) {
        QPoint p(pos());
        QSize s(size());
        _prefs->set(QString("%1.x").arg(name), p.x());
        _prefs->set(QString("%1.y").arg(name), p.y());
        _prefs->set(QString("%1.w").arg(name), s.width());
        _prefs->set(QString("%1.h").arg(name), s.height());
    }

    super::hideEvent(event);
}

void zcVideoDock::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange) {
        bool fscr;
        if ((this->windowState() & Qt::WindowFullScreen) != 0) {
            fscr = true;
        } else {
            fscr = false;
        }

        D->_video_widget->handleDockChange(fscr);

        e->accept();
    }


}



