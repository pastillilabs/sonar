TARGET = $${HARBOUR_NAME}
VERSION = 0.0.6

QT -= gui
QT += dbus network

CONFIG -= app_bundle
CONFIG += c++11
CONFIG += console
CONFIG += link_pkgconfig

PKGCONFIG += KF5CalendarCore
PKGCONFIG += libical
PKGCONFIG += libmkcal-qt5
PKGCONFIG += Qt5Contacts

QMAKE_RPATHDIR += /usr/share/$${HARBOUR_NAME}/lib

DEFINES += QT_NO_CAST_FROM_ASCII
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_FOREACH
DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT

INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/../jsonipc/include
INCLUDEPATH += /usr/include/kcalcoren-qt5
INCLUDEPATH += /usr/include/mkcal-qt5

HEADERS += \
    src/client.h \
    src/platform.h \
    src/server.h \
    src/types.h \
    src/observers/calendar.h

SOURCES += \
    src/main.cpp \
    src/client.cpp \
    src/platform.cpp \
    src/server.cpp \
    src/observers/calendar.cpp

DISTFILES += \
    harbour-situations2application.service \
    situations-sonar.service

LIBS += -L$$OUT_PWD/../jsonipc -ljsonipc

SONAR_HOST_ARCH = $${QMAKE_HOST.arch}

target.path = /usr/bin
INSTALLS += target

situations-sonar-service.files = situations-sonar.service
situations-sonar-service.path = /etc/systemd/system
INSTALLS += situations-sonar-service

situations-service.files = harbour-situations2application.service
situations-service.path = /usr/lib/systemd/user
INSTALLS += situations-service
