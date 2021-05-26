/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#include "zcvideowidgetslider.h"

#include <QMouseEvent>


zcVideoWidgetSlider::zcVideoWidgetSlider(QWidget *parent)
    : zcVideoWidgetSlider(Qt::Vertical, parent)
{
}

zcVideoWidgetSlider::zcVideoWidgetSlider(Qt::Orientation o, QWidget *parent)
    : QSlider(o, parent)
{
    this->setTracking(false);
}

void zcVideoWidgetSlider::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    int w = width();
    qreal factor = static_cast<qreal>(this->sliderPosition()) / maximum();

    int slider_pos = factor * w;

    if (pos.x() > (slider_pos + 10)) {
        triggerAction(QSlider::SliderSingleStepAdd);
    } else if (pos.x() < (slider_pos - 10)) {
        triggerAction(QSlider::SliderSingleStepSub);
    }

    QSlider::mouseReleaseEvent(event);
}
