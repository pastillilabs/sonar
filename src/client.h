#pragma once

#include <QLocalSocket>
#include <QVariantMap>

namespace sonar {
namespace client {

void receive(const QVariantMap& message, QLocalSocket& from);
void send(const QVariantMap& message, QLocalSocket& to);

} // namespace client
} // namespace sonar
