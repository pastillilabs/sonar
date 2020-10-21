#pragma once

#include <jsonclient.h>
#include <QJsonValue>

namespace sonar {
namespace client {

void receive(JsonClient& jsonClient, const QJsonObject& message);

} // namespace client
} // namespace sonar
