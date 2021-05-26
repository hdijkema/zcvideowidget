/*
 * https://github.com/hdijkema/zcvideowidget
 *
 * The zcVideoWidget library provides a simple VideoWidget and Video Dock / Floating window
 * for Qt applications. It is great to use together with the ffmpeg-plugin for QMultiMedia
 * See https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg.
 *
 * Copyright (C) 2021 Hans Dijkema, License LGPLv3
 */

#ifndef ZCVIDEOFLAGS_H
#define ZCVIDEOFLAGS_H

class zcVideoFlags
{
    // zcVideoWidget flags
public:
    static const int FLAG_SOFT_TITLE             = 0x0001;

    // zcVideoDock flags
public:
    static const int FLAG_KEEP_POSITION_AND_SIZE = 0x0100;

    // Internal flags
private:
    static const int FLAG_DOCKED = 0x0002;

public:
    friend class zcVideoWidget;
    friend class zcVideoDock;
};

#endif // ZCVIDEOFLAGS_H
