#include "server.h"
#include "client.h"

#include <QDebug>
#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QList>
#include <QLocalServer>
#include <QLocalSocket>

#include <sys/socket.h>
#include <sys/types.h>

namespace {

const QString SERVER_URI{QStringLiteral("com.pastillilabs.sonar")};
const QString PROC_CMDLINE{QStringLiteral("/proc/%1/cmdline")};
const QByteArray REQUIRED_CMDLINE{"/usr/bin/harbour-situations2application"};

QLocalServer localServer;
QHash<QLocalSocket*, qint64> clientBufferSizes;

void disconnected(QLocalSocket* socket)
{
    clientBufferSizes.take(socket);
    socket->deleteLater();

    qInfo() << "Connection closed";
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
            sonar::client::receive(map, *socket);
        }
    }
}

bool acceptConnection(QLocalSocket* socket) {
    bool accepted(false);

    // Check if connection can be accepted
    if(socket != nullptr) {
        ucred ucred;
        socklen_t len = static_cast<socklen_t>(sizeof(ucred));
        if(getsockopt(socket->socketDescriptor(), SOL_SOCKET, SO_PEERCRED, &ucred, &len) != -1) {
            QFile processCmdlineFile(PROC_CMDLINE.arg(ucred.pid));
            if(processCmdlineFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QList<QByteArray> cmdLine(processCmdlineFile.readAll().split('\0'));

                if(cmdLine.first() == REQUIRED_CMDLINE) {
                    qInfo() << "Connection accepted";
                    accepted = true;
                }
                else {
                    qCritical() << "Connection refused: client not accepted!";
                }
            }
        }
        else {
            qCritical() << "Connection refused: getting required client information failed!";
        }
    }

    return accepted;
}

void newConnection()
{
    QLocalSocket* socket(localServer.nextPendingConnection());
    if(acceptConnection(socket)) {
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
namespace server {

bool start()
{
    bool success(false);

    // Make sure no multiple instances are running
    QLocalSocket socket;
    socket.connectToServer(SERVER_URI);
    if(socket.waitForConnected(1000)) {
        socket.disconnectFromServer();

        qCritical() << "Server already running";
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

        if(startCounter) {
            success = true;
        }
        else {
            qCritical() << "Unable to open local socket:" << localServer.errorString();
        }
    }

    return success;
}

} // namespace server
} // namespace sonar
