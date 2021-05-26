/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#ifndef ZCVIDEODOCK_H
#define ZCVIDEODOCK_H

#include <QDockWidget>
#include "zcvideowidget.h"
#include "zcvideoflags.h"

class ZCVIDEOWIDGET_EXPORT zcVideoDock : public QDockWidget
{
    Q_OBJECT
private:
    zcVideoWidget           *_video_widget;
    zcVideoWidget::Prefs    *_prefs; // will be owned and deleted by zcVideoWidget
    int                      _flags;

public:
    explicit zcVideoDock(zcVideoWidget::Prefs *p, int flags, QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());
    explicit zcVideoDock(zcVideoWidget::Prefs *p, QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());
    explicit zcVideoDock(int flags, QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());
    explicit zcVideoDock(QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());

public:
    bool hasPositionAndSize();

public:
    zcVideoWidget *videoWidget();

public:
    void setObjectName(const QString &name);

    // QWidget interface
protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
};


#endif // ZCVIDEODOCK_H
