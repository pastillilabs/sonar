#pragma once

#include "types.h"

namespace sonar {
namespace platform {

QJsonValue getCalendars(const QJsonValue& payload);
QJsonValue getCalendarEvents(const QJsonValue& payload);
QJsonValue getContacts(const QJsonValue& payload);
QJsonValue getSupportedPermissions(const QJsonValue& payload);

void registerCalendarChangeObserver(JsonClient& jsonClient, const QJsonValue& payload);
void unregisterCalendarChangeObserver(JsonClient& jsonClient);

void setBluetoothState(const QJsonValue& payload);
void setWifiState(const QJsonValue& payload);
void setCellularState(const QJsonValue& payload);
void setCellularRadioTechnology(const QJsonValue& payload);
void setFlightmodeState(const QJsonValue& payload);
void setWifiTethering(const QJsonValue& payload);

void commandSendSms(const QJsonValue& payload);
void commandUninstall(const QJsonValue& payload);

} // namespace platform
} // namespace sonar
