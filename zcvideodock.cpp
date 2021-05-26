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

zcVideoDock::zcVideoDock(QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(nullptr, 0, parent, wflags)
{}

bool zcVideoDock::hasPositionAndSize()
{
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

zcVideoDock::zcVideoDock(zcVideoWidget::Prefs *p, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(p, 0, parent, wflags)
{}

zcVideoDock::zcVideoDock(int flags, QWidget *parent, Qt::WindowFlags wflags)
    : zcVideoDock(nullptr, flags, parent, wflags)
{}

zcVideoDock::zcVideoDock(zcVideoWidget::Prefs *p, int flags, QWidget *parent, Qt::WindowFlags wflags)
    : QDockWidget(parent, wflags)
{
    _prefs = p;
    _flags = flags;
    _video_widget = new zcVideoWidget(p, flags|zcVideoFlags::FLAG_DOCKED, this);
    setWidget(_video_widget);
}

zcVideoWidget *zcVideoDock::videoWidget()
{
    return _video_widget;
}

void zcVideoDock::setObjectName(const QString &name)
{
    QDockWidget::setObjectName(name);
    _video_widget->setObjectName(name);
}

void zcVideoDock::showEvent(QShowEvent *event)
{
    QString objn = objectName();
    if (objn == "") { objn = "default"; }
    QString name = QString("zcVideoDock.%1").arg(objn);
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
    QDockWidget::showEvent(event);
}

void zcVideoDock::hideEvent(QHideEvent *event)
{
    QString objn = objectName();
    if (objn == "") { objn = "default"; }
    QString name = QString("zcVideoDock.%1").arg(objn);
    if (_prefs && _flags&zcVideoFlags::FLAG_KEEP_POSITION_AND_SIZE) {
        QPoint p(pos());
        QSize s(size());
        _prefs->set(QString("%1.x").arg(name), p.x());
        _prefs->set(QString("%1.y").arg(name), p.y());
        _prefs->set(QString("%1.w").arg(name), s.width());
        _prefs->set(QString("%1.h").arg(name), s.height());
    }
    QDockWidget::hideEvent(event);
}



