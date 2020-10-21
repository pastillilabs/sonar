#pragma once

#include "types.h"
#include <jsonclient.h>
#include <QJsonValue>

namespace sonar {
namespace observers {
namespace calendar {

void registerChangeObserver(JsonClient& jsonClient, const QJsonValue& payload);
void unregisterChangeObserver(JsonClient& jsonClient);

} // namespace calendar
} // namespace observers
} // namespace sonar
