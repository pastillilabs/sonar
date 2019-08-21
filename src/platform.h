#ifndef PLATFORM_H
#define PLATFORM_H

#include <QObject>

namespace sonar {
namespace platform {

void setBluetoothState(const QVariant& value);
void setWifiState(const QVariant& value);
void setCellularState(const QVariant& value);
void setCellularRadioTechnology(const QVariant& value);
void setFlightmodeState(const QVariant& value);

} // namespace platform
} // namespace sonar

#endif // PLATFORM_H
