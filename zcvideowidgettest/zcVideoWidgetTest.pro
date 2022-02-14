QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

mac: message(Qt Deployment Target: $$QMAKE_MACOSX_DEPLOYMENT_TARGET)
mac: QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

CONFIG += c++11
DEFINES += ZCVIDEOWIDGETTEST

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ..

SOURCES += \
    ../zcvideodock.cpp \
    ../zcvideowidget.cpp \
    ../zcvideowidgetslider.cpp \
    ../zcvideowidgetsrtparser.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ../zcvideodock.h \
    ../zcvideoflags.h \
    ../zcvideowidget.h \
    ../zcvideowidget_global.h \
    ../zcvideowidgetslider.h \
    ../zcvideowidgetsrtparser.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
