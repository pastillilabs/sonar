TARGET = situations-sonar
VERSION = 0.0.5

QT -= gui
QT += dbus network

CONFIG -= app_bundle
CONFIG += c++11
CONFIG += console
CONFIG += link_pkgconfig

PKGCONFIG += libical
PKGCONFIG += libkcalcoren-qt5
PKGCONFIG += libmkcal-qt5
PKGCONFIG += Qt5Contacts

DEFINES += QT_NO_CAST_FROM_ASCII
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_FOREACH
DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT

INCLUDEPATH += $$PWD/src
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
    rpm/situations-sonar.changes \
    rpm/situations-sonar.spec \
    rpm/situations-sonar.yaml \
    service/harbour-situations2application.service \
    service/situations-sonar.service

target.path = /usr/bin
INSTALLS += target

situations-sonar-service.files = service/situations-sonar.service
situations-sonar-service.path = /etc/systemd/system
INSTALLS += situations-sonar-service

situations-service.files = service/harbour-situations2application.service
situations-service.path = /usr/lib/systemd/user
INSTALLS += situations-service

