QT += widgets multimedia multimediawidgets

TEMPLATE = lib
DEFINES += ZCVIDEOWIDGET_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    zcvideodock.cpp \
    zcvideowidget.cpp \
    zcvideowidgetslider.cpp

HEADERS += \
    srtparser.h \
    zcvideowidget \
    zcvideodock.h \
    zcvideoflags.h \
    zcvideowidget.h \
    zcvideowidget_global.h \
    zcvideowidgetslider.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md
