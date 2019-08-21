#include "messenger.h"
#include "request/set.h"
#include <QDebug>
#include <QHash>
#include <functional>

namespace {

// Message fields
const QString REQUEST{QStringLiteral("request")};

// Requests
const QString REQUEST_SET{QStringLiteral("set")};

// request handlers
QHash<QString, std::function<void(const QVariantMap&, QLocalSocket&)>> receiveHandlers{
    { REQUEST_SET, sonar::request::set }
};

} // namespace

namespace sonar {

void receive(const QVariantMap& message, QLocalSocket& from)
{
    const QString request(message.value(REQUEST).toString());

    receiveHandlers.value(request, [request](const QVariantMap& /*message*/, QLocalSocket& /*from*/) {
        qWarning() << "No request handler for" << request;
    })(message, from);
}

} // namespace sonar
