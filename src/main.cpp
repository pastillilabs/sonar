#include "server.h"
#include <QCoreApplication>
#include <QDebug>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    setenv("HOME", "/home/nemo", 1);
    setenv("USER", "nemo", 1);

    int result(1);
    if(sonar::server::start()) {
        result = app.exec();
    }

    return result;
}
