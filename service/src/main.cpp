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

    // Currently Sonar works only for nemo / defaultuser
    const QVersionNumber productVersion{QVersionNumber::fromString(QSysInfo::productVersion())};
    if(productVersion > QVersionNumber(3, 4) && QDir(QStringLiteral("/home/defaultuser")).exists()) {
        setenv("HOME", "/home/defaultuser", 1);
        setenv("USER", "defaultuser", 1);
    }
    else {
        setenv("HOME", "/home/nemo", 1);
        setenv("USER", "nemo", 1);
    }

    int result(1);
    if(sonar::server::start()) {
        result = app.exec();
    }

    return result;
}
