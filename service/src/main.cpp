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
    if(productVersion > QVersionNumber(3, 4) {
        setenv("HOME", QDir::homePath(), 1);            // homePath() returns absolute path
        setenv("USER", QDir::home().dirName(), 1);      // home.dirName() returns last path segment
    }

    int result(1);
    if(sonar::server::start()) {
        result = app.exec();
    }

    return result;
}
