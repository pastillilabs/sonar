#include "server.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSysInfo>
#include <QVersionNumber>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QVersionNumber productVersion{QVersionNumber::fromString(QSysInfo::productVersion())};
    const QVersionNumber checkVersion = QVersionNumber(3, 4);
    if(productVersion > checkVersion) {
        setenv("HOME", QDir::homePath().toLocal8Bit().constData(), 1);            // homePath() returns absolute path
        setenv("USER", QDir::home().dirName().toLocal8Bit().constData(), 1);      // home.dirName() returns last path segment
    }

    int result(1);
    if(sonar::server::start()) {
        result = app.exec();
    }

    return result;
}
