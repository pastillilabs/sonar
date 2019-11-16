#pragma once

#include "types.h"

namespace sonar {
namespace platform {

QVariant getCalendars(const QVariant& payload);
QVariant getCalendarEvents(const QVariant& payload);
QVariant getSupportedPermissions(const QVariant& payload);

void registerCalendarChangeObserver(const QVariant& payload, QLocalSocket& client, Notifier notifier);
void unregisterCalendarChangeObserver(QLocalSocket& client);

void setBluetoothState(const QVariant& payload);
void setWifiState(const QVariant& payload);
void setCellularState(const QVariant& payload);
void setCellularRadioTechnology(const QVariant& payload);
void setFlightmodeState(const QVariant& payload);
void setWifiTethering(const QVariant& payload);

void commandUninstall(const QVariant& payload);

} // namespace platform
} // namespace sonar
