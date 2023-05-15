#
# Install:
#
# Copy all .h and zcVideoWidget files to your include directory
# Copy lib and dll files to the appropriate directories
#


QT += widgets multimedia multimediawidgets

TEMPLATE = lib
DEFINES += ZCVIDEOWIDGET_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

equals(QT_MAJOR_VERSION, 6) {
win32: TARGET = zcvideowidget6
mac: TARGET = libzcvideowidget6
linux: TARGET = libzcvideowidget6
}

mac: message(Qt Deployment Target: $$QMAKE_MACOSX_DEPLOYMENT_TARGET)
mac: QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

include(zcvideowidget.pri)

mac: PREFIX=/Users/hans/devel/libraries/nosx
win32: PREFIX=c:/devel/libraries/nwin64
linux: PREFIX=/home/hans/devel/libraries/linux64

LIB_TARGET=lib
BIN_TARGET=lib
CONFIG(debug, debug|release) {
    win32: LIB_TARGET=libd
    win32: BIN_TARGET=libd
    KIND_TARGET=debug
} else {
    win32:LIB_TARGET=lib
    win32:BIN_TARGET=bin
    KIND_TARGET=release
}

win32: target.path = $$PREFIX/$$BIN_TARGET
win32: target.files = $$OUT_PWD/$$KIND_TARGET/*.dll
linux: target.path = $$PREFIX/$$LIB_TARGET
mac: target.path = $$PREFIX/$$LIB_TARGET

INSTALLS += target

win32: libtarget.path = $$PREFIX/$$LIB_TARGET
win32: libtarget.files = $$OUT_PWD/$$KIND_TARGET/*.lib $$OUT_PWD/$$KIND_TARGET/*.exp
CONFIG(debug, debug|release) {
win32: libtarget.files += $$OUT_PWD/$$KIND_TARGET/*.pdb $$OUT_PWD/$$KIND_TARGET/*.ilk
}
win32: INSTALLS += libtarget

#### Headers to install
headers.path = $$PREFIX/include
headers.files = zcvideodock.h zcvideoflags.h zcvideowidget.h zcvideowidgetslider.h zcvideowidget_global.h zcVideoWidget

INSTALLS += headers

message(installs = $$INSTALLS)


DISTFILES += \
    LICENSE \
    README.md \
    zcvideowidget.pri
