#include "request/set.h"
#include "platform.h"
#include <QDebug>
#include <QHash>
#include <functional>

namespace {
// Message fields
const QString TARGET{QStringLiteral("target")};
const QString VALUE{QStringLiteral("value")};

// Targets
const QString TARGET_BLUETOOTH_STATE{QStringLiteral("bluetooth_state")};
const QString TARGET_WIFI_STATE{QStringLiteral("wifi_state")};
const QString TARGET_CELLULAR_STATE{QStringLiteral("cellular_state")};
const QString TARGET_CELLULAR_RADIO_TECHNOLOGY{QStringLiteral("cellular_radio_technology")};
const QString TARGET_FLIGHTMODE_STATE{QStringLiteral("flightmode_state")};

// set handlers
QHash<QString, std::function<void(const QVariant&)>> setHandlers{
    { TARGET_BLUETOOTH_STATE, sonar::platform::setBluetoothState },
    { TARGET_WIFI_STATE, sonar::platform::setWifiState },
    { TARGET_CELLULAR_STATE, sonar::platform::setCellularState },
    { TARGET_CELLULAR_RADIO_TECHNOLOGY, sonar::platform::setCellularRadioTechnology },
    { TARGET_FLIGHTMODE_STATE, sonar::platform::setFlightmodeState },
};
} // namespace

namespace sonar {
namespace request {

void set(const QVariantMap& message, QLocalSocket& /*from*/) {
    const QString target(message.value(TARGET).toString());

    setHandlers.value(target, [target](const QVariant& /*value*/) {
        qWarning() << "No setter for" << target;
    })(message.value(VALUE));
}

} // namespace request
} // namespace sonar
