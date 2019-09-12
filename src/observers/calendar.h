#pragma once

#include "types.h"

namespace sonar {
namespace observers {
namespace calendar {

void registerChangeObserver(const QVariant& payload, QLocalSocket& client, Notifier notifier);
void unregisterChangeObserver(QLocalSocket& client);

} // namespace calendar
} // namespace observers
} // namespace sonar
