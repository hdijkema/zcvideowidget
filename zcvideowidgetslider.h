/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#ifndef ZCVIDEOWIDGETSLIDER_H
#define ZCVIDEOWIDGETSLIDER_H

#include "zcvideowidget_global.h"
#include <QSlider>

class ZCVIDEOWIDGET_EXPORT zcVideoWidgetSlider : public QSlider
{
    Q_OBJECT
public:
    zcVideoWidgetSlider(QWidget *parent = nullptr);
    zcVideoWidgetSlider(Qt::Orientation o, QWidget *parent = nullptr);

// QWidget interface
protected:
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // ZCVIDEOWIDGETSLIDER_H
