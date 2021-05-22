#ifndef ZCVIDEOWIDGETSLIDER_H
#define ZCVIDEOWIDGETSLIDER_H

#include "zcvideowidget_global.h"
#include <QSlider>
#include <QWidget>

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
