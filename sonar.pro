TARGET = situations-sonar
VERSION = 0.0.1

QT -= gui
QT += dbus network

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_NO_CAST_FROM_ASCII
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_FOREACH
DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT

INCLUDEPATH += $$PWD/src

HEADERS += \
    src/messenger.h \
    src/platform.h \
    src/server.h \
    src/request/set.h

SOURCES += \
    src/main.cpp \
    src/messenger.cpp \
    src/platform.cpp \
    src/server.cpp \
    src/request/set.cpp

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

