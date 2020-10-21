#include "server.h"
#include "client.h"

#include <jsonserver.h>
#include <QDebug>
#include <QFile>
#include <QList>
#include <sys/socket.h>
#include <sys/types.h>

namespace {

const QString SERVER_URI{QStringLiteral("com.pastillilabs.sonar")};
const QString PROC_CMDLINE{QStringLiteral("/proc/%1/cmdline")};
const QByteArray REQUIRED_CMDLINE{"/usr/bin/harbour-situations2application"};

JsonServer jsonServer;

const JsonServer::AcceptHandler acceptHandler = [](QLocalSocket& socket) {
    bool accepted(false);

    // Check if connection can be accepted
    ucred ucred;
    socklen_t len = static_cast<socklen_t>(sizeof(ucred));
    if(getsockopt(socket.socketDescriptor(), SOL_SOCKET, SO_PEERCRED, &ucred, &len) != -1) {
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

    return accepted;
};


} // namespace

namespace sonar {
namespace server {

bool start()
{
    QObject::connect(&jsonServer, &JsonServer::newClient,  [](JsonClient& jsonClient) {
        QObject::connect(&jsonClient, &JsonClient::received, [&jsonClient](const QJsonObject& message) {
            sonar::client::receive(jsonClient, message);
        });
    });

    jsonServer.setAcceptHandler(acceptHandler);
    jsonServer.setSocketOptions(QLocalServer::WorldAccessOption);

    return jsonServer.listen(SERVER_URI);
}

} // namespace server
} // namespace sonar
