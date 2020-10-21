#include "client.h"
#include "platform.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

// Get handlers
const QHash<QString, sonar::GetHandler> getHandlers{
    { QLatin1String("calendars"), sonar::platform::getCalendars },
    { QLatin1String("calendarEvents"), sonar::platform::getCalendarEvents },
    { QLatin1String("contacts"), sonar::platform::getContacts },
    { QLatin1String("supportedPermissions"), sonar::platform::getSupportedPermissions }
};

// Set handlers
const QHash<QString, sonar::SetHandler> setHandlers{
    { QLatin1String("bluetoothState"), sonar::platform::setBluetoothState },
    { QLatin1String("cellularState"), sonar::platform::setCellularState },
    { QLatin1String("cellularRadioTechnology"), sonar::platform::setCellularRadioTechnology },
    { QLatin1String("flightmodeState"), sonar::platform::setFlightmodeState },
    { QLatin1String("wifiState"), sonar::platform::setWifiState },
    { QLatin1String("wifiTethering"), sonar::platform::setWifiTethering }
};

// Register handlers
const QHash<QString, sonar::RegisterHandler> registerHandlers{
    { QLatin1String("calendarChange"), sonar::platform::registerCalendarChangeObserver }
};

// Unregister handlers
const QHash<QString, sonar::UnregisterHandler> unregisterHandlers{
    { QLatin1String("calendarChange"), sonar::platform::unregisterCalendarChangeObserver }
};

// Command handlers
const QHash<QString, sonar::CommandHandler> commandHandlers{
    { QLatin1String("send_sms"), sonar::platform::commandSendSms },
    { QLatin1String("uninstall"), sonar::platform::commandUninstall }
};

void handleGet(JsonClient& jsonClient, const QJsonObject& message)
{
    const QString target(message.value(QLatin1String("target")).toString());

    const sonar::GetHandler defaultHandler = [target](const QJsonValue&) -> QJsonValue {
        qWarning() << "No get handler for" << target;
        return QJsonValue();
    };

    const sonar::GetHandler handler = getHandlers.value(target, defaultHandler);
    const QJsonObject response{
        { QLatin1String("method"), QLatin1String("return") },
        { QLatin1String("target"), target },
        { QLatin1String("payload"), handler(message.value(QLatin1String("payload"))) }
    };

    jsonClient.send(response);
}

void handleSet(const QJsonObject& message)
{
    const QString target(message.value(QLatin1String("target")).toString());

    const sonar::SetHandler defaultHandler = [target](const QVariant&) {
        qWarning() << "No set handler for" << target;
    };

    const sonar::SetHandler handler = setHandlers.value(target, defaultHandler);
    handler(message.value(QLatin1String("payload")));
}

void handleRegister(JsonClient& jsonClient, const QJsonObject& message)
{
    const QString target(message.value(QLatin1String("target")).toString());

    const sonar::RegisterHandler defaultHandler = [target](JsonClient&, const QJsonValue&) {
        qWarning() << "No register handler for" << target;
    };

    const sonar::RegisterHandler handler = registerHandlers.value(target, defaultHandler);
    handler(jsonClient, message.value(QLatin1String("payload")));
}

void handleUnregister(JsonClient& jsonClient, const QJsonObject& message)
{
    const QString target(message.value(QLatin1String("target")).toString());

    const sonar::UnregisterHandler defaultHandler = [target](JsonClient&) {
        qWarning() << "No unregister handler for" << target;
    };

    const sonar::UnregisterHandler handler = unregisterHandlers.value(target, defaultHandler);
    handler(jsonClient);
}

void handleCommand(const QJsonObject& message)
{
    const QString target(message.value(QLatin1String("target")).toString());

    const sonar::CommandHandler defaultHandler = [target](const QVariant&) {
        qWarning() << "No command handler for" << target;
    };

    const sonar::CommandHandler handler = commandHandlers.value(target, defaultHandler);
    handler(message.value(QLatin1String("payload")));
}

} // namespace

namespace sonar {
namespace client {

void receive(JsonClient& jsonClient, const QJsonObject& message)
{
    const QString method(message.value(QLatin1String("method")).toString());

    if(method == QLatin1String("get")) {
        handleGet(jsonClient, message);
    }
    else if(method == QLatin1String("set")) {
        handleSet(message);
    }
    else if(method == QLatin1String("register")) {
        handleRegister(jsonClient, message);
    }
    else if(method == QLatin1String("unregister")) {
        handleUnregister(jsonClient, message);
    }
    else if(method == QLatin1String("command")) {
        handleCommand(message);
    }
    else {
        qWarning() << "Unknown method" << method;
    }
}

} // namespace client
} // namespace sonar
