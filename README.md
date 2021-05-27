# zcVideoWidget

zcVideoWidget is a library that provides a QVideoWidget with controls.

## Has following features

* Floating window.
* Storing preferences in via a support class.
* SubRip subtitles (.srt files).
* Full Screen mode when used in a zcVideoDock.
* Hiding the cursor / controls while in fullscreen mode. 
* Preventing the operatingsystem from going to sleep (including display) while in fullscreen mode.

## License

The license for this widget is LGPLv3.

## Backend

This widget works with the normal Qt MultiMedia backend. 

However, see also the [ffmpeg-plugin](https://github.com/hdijkema/qtmultimedia-plugin-ffmpeg), for upgrading the supported video- and audiocodecs of the Qt MultiMedia framework. 
