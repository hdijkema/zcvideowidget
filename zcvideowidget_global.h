/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#ifndef ZCVIDEOWIDGET_GLOBAL_H
#define ZCVIDEOWIDGET_GLOBAL_H

#include <QtCore/qglobal.h>


#if defined(ZCVIDEOWIDGET_LIBRARY)
#  define ZCVIDEOWIDGET_EXPORT Q_DECL_EXPORT
#else
#  define ZCVIDEOWIDGET_EXPORT Q_DECL_IMPORT
#endif


#endif // ZCVIDEOWIDGET_GLOBAL_H
