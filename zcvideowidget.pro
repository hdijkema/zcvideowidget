QT += widgets multimedia multimediawidgets

TEMPLATE = lib
DEFINES += ZCVIDEOWIDGET_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

equals(QT_MAJOR_VERSION, 5) {
win32: TARGET = zcvideowidget5
mac: TARGET = libzcvideowidget5
}

equals(QT_MAJOR_VERSION, 6) {
win32: TARGET = zcvideowidget6
mac: TARGET = libzcvideowidget6
}

SOURCES += \
    zcvideodock.cpp \
    zcvideowidget.cpp \
    zcvideowidgetslider.cpp \
    zcvideowidgetsrtparser.cpp

HEADERS += \
    zcvideowidget \
    zcvideodock.h \
    zcvideoflags.h \
    zcvideowidget.h \
    zcvideowidget_global.h \
    zcvideowidgetslider.h \
    zcvideowidgetsrtparser.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    LICENSE \
    README.md
