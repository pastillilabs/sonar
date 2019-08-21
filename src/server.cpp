#include "server.h"
#include "messenger.h"
#include <QCoreApplication>
#include <QDebug>
#include <QHash>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>

namespace {

const QString SERVER_URI{QStringLiteral("com.pastillilabs.sonar")};

QLocalServer localServer;
QHash<QLocalSocket*, qint64> clientBufferSizes;

void disconnected(QLocalSocket* socket)
{
    clientBufferSizes.take(socket);
    socket->deleteLater();
}

void readyRead(QLocalSocket* socket)
{
    bool bufferReceived(true);
    while(!socket->atEnd() && bufferReceived) {
        qint64 bufferSize(clientBufferSizes.value(socket));

        // Read incoming message size if not already known
        if(bufferSize < 0 && socket->bytesAvailable() >= static_cast<qint64>(sizeof(bufferSize))) {
            socket->read(reinterpret_cast<char*>(&bufferSize), static_cast<qint64>(sizeof(bufferSize)));
            clientBufferSizes[socket] = bufferSize;
        }

        // Read buffer if fully available
        bufferReceived = (socket->bytesAvailable() >= bufferSize);
        if(bufferReceived) {
            const QByteArray buffer(socket->read(bufferSize));
            clientBufferSizes[socket] = -1;

            const QVariantMap map(QJsonDocument::fromJson(buffer).toVariant().toMap());
            sonar::receive(map, *socket);
        }
    }
}

void newConnection()
{
    QLocalSocket* socket(localServer.nextPendingConnection());
    if(socket) {
        clientBufferSizes[socket] = -1;

        QObject::connect(socket, &QLocalSocket::disconnected, [socket] {
            disconnected(socket);
        });

        QObject::connect(socket, &QLocalSocket::readyRead, [socket] {
            readyRead(socket);
        });
    }
}

} // namespace

namespace sonar {

void startServer()
{
    // Make sure no multiple instances are running
    QLocalSocket socket;
    socket.connectToServer(SERVER_URI);
    if(socket.waitForConnected(1000)) {
        socket.disconnectFromServer();

        qCritical() << "Server already running";
        QCoreApplication::exit(1);
    }
    else {
        localServer.setSocketOptions(QLocalServer::WorldAccessOption);

        QObject::connect(&localServer, &QLocalServer::newConnection, &newConnection);

        // Start service with max 3 retries
        int startCounter(3);
        while(startCounter && !localServer.listen(SERVER_URI)) {
            --startCounter;
            QLocalServer::removeServer(SERVER_URI);
        }

        // Finish on unsuccessful start
        if(!startCounter) {
            qCritical() << "Unable to open local socket:" << localServer.errorString();
            QCoreApplication::exit(1);
        }
    }
}

} // namespace sonar
