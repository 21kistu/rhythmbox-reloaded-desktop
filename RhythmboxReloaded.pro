QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    application-src/main.cpp \
    application-src/mainwindow.cpp \
    application-src/newtrackselector.cpp \
    application-src/playlistmanager.cpp \
    application-src/spectralview.cpp \
    libraries/tinyxml2.cpp \
    application-src/trackitem.cpp

HEADERS += \
    application-headers/mainwindow.h \
    libraries/miniaudio.h \
    application-headers/newtrackselector.h \
    application-headers/playlistmanager.h \
    application-headers/roundedpixmap.h \
    application-headers/spectralview.h \
    libraries/tinyxml2.h \
    application-headers/trackitem.h \
    application-headers/communicate.h

FORMS += \
    application-forms/mainwindow.ui \
    application-forms/newtrackselector.ui \
    application-forms/trackitem.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += taglib

INCLUDEPATH += application-headers/
INCLUDEPATH += application-src/
INCLUDEPATH += application-forms/
INCLUDEPATH += libraries/

DISTFILES +=

RESOURCES += \
    application-styling/application-styling.qrc
