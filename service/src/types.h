#pragma once

#include <jsonclient.h>
#include <functional>
#include <QJsonValue>

namespace sonar {

using GetHandler = std::function<QJsonValue(const QJsonValue&)>;
using SetHandler = std::function<void(const QJsonValue&)>;
using RegisterHandler = std::function<void(JsonClient&, const QJsonValue&)>;
using UnregisterHandler = std::function<void(JsonClient&)>;
using CommandHandler = std::function<void(const QJsonValue&)>;

} // namespace sonar
