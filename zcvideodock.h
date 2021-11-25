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

#include "zcvideowidget_global.h"
#include <QMainWindow>
#include "zcvideowidget.h"

class zcVideoDockData;

class ZCVIDEOWIDGET_EXPORT zcVideoDock : public QMainWindow
{
    Q_OBJECT
private:
    zcVideoDockData         *D;

public:
    explicit zcVideoDock(zcVideoWidget::Prefs *p, int flags, QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());
    explicit zcVideoDock(zcVideoWidget::Prefs *p, QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());
    explicit zcVideoDock(int flags, QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());
    explicit zcVideoDock(QWidget *parent = nullptr, Qt::WindowFlags wflags = Qt::WindowFlags());
    // zcVideoWidget::Prefs *p will be owned and deleted by the underlying zcVideoWidget

    ~zcVideoDock();

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
    virtual void changeEvent(QEvent *e) override;
};


#endif // ZCVIDEODOCK_H
