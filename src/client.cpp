#include "client.h"
#include "platform.h"

#include <QDebug>
#include <QJsonDocument>

namespace {

// Message fields
const QString METHOD{QStringLiteral("method")};
const QString TARGET{QStringLiteral("target")};
const QString PAYLOAD{QStringLiteral("payload")};

// Methods
const QString GET(QStringLiteral("get"));
const QString SET(QStringLiteral("set"));
const QString REGISTER(QStringLiteral("register"));
const QString UNREGISTER(QStringLiteral("unregister"));
const QString COMMAND(QStringLiteral("command"));

const QString RETURN(QStringLiteral("return"));
const QString NOTIFY(QStringLiteral("notify"));

// Targets
const QString CALENDARS{QStringLiteral("calendars")};
const QString CALENDAR_EVENTS{QStringLiteral("calendarEvents")};
const QString SUPPORTED_PERMISSIONS{QStringLiteral("supportedPermissions")};

const QString BLUETOOTH_STATE{QStringLiteral("bluetoothState")};
const QString CELLULAR_STATE{QStringLiteral("cellularState")};
const QString CELLULAR_RADIO_TECHNOLOGY{QStringLiteral("cellularRadioTechnology")};
const QString FLIGHTMODE_STATE{QStringLiteral("flightmodeState")};
const QString WIFI_STATE{QStringLiteral("wifiState")};

const QString CALENDAR_CHANGE{QStringLiteral("calendarChange")};

const QString UNINSTALL{QStringLiteral("uninstall")};

// Calendar changed notifier
const sonar::Notifier calendarChangeNotifier = [](QLocalSocket& client, const QVariant& payload) {
    const QVariantMap message{
        { METHOD, NOTIFY },
        { TARGET, CALENDAR_CHANGE },
        { PAYLOAD, payload }
    };

    sonar::client::send(message, client);
};

// Get handlers
const QHash<QString, sonar::GetHandler> GET_HANDLERS{
    { CALENDARS, sonar::platform::getCalendars },
    { CALENDAR_EVENTS, sonar::platform::getCalendarEvents },
    { SUPPORTED_PERMISSIONS, sonar::platform::getSupportedPermissions }
};

// Set handlers
const QHash<QString, sonar::SetHandler> SET_HANDLERS{
    { BLUETOOTH_STATE, sonar::platform::setBluetoothState },
    { CELLULAR_STATE, sonar::platform::setCellularState },
    { CELLULAR_RADIO_TECHNOLOGY, sonar::platform::setCellularRadioTechnology },
    { FLIGHTMODE_STATE, sonar::platform::setFlightmodeState },
    { WIFI_STATE, sonar::platform::setWifiState }
};

// Register handlers
const QHash<QString, sonar::RegisterHandler> REGISTER_HANDLERS{
    { CALENDAR_CHANGE, sonar::platform::registerCalendarChangeObserver }
};

// Unregister handlers
const QHash<QString, sonar::UnregisterHandler> UNREGISTER_HANDLERS{
    { CALENDAR_CHANGE, sonar::platform::unregisterCalendarChangeObserver }
};

// Command handlers
const QHash<QString, sonar::CommandHandler> COMMAND_HANDLERS{
    { UNINSTALL, sonar::platform::commandUninstall }
};

// Notifiers
const QHash<QString, sonar::Notifier> NOTIFIERS{
    { CALENDAR_CHANGE, calendarChangeNotifier }
};

void handleGet(const QVariantMap& message, QLocalSocket& from)
{
    const QString target(message.value(TARGET).toString());

    const sonar::GetHandler defaultHandler = [target](const QVariant&) -> QVariant {
        qWarning() << "No get handler for" << target;
        return QVariant();
    };

    const sonar::GetHandler handler = GET_HANDLERS.value(target, defaultHandler);
    const QVariantMap response{
        { METHOD, RETURN },
        { TARGET, target },
        { PAYLOAD, handler(message.value(PAYLOAD)) }
    };

    sonar::client::send(response, from);
}

void handleSet(const QVariantMap& message)
{
    const QString target(message.value(TARGET).toString());

    const sonar::SetHandler defaultHandler = [target](const QVariant&) {
        qWarning() << "No set handler for" << target;
    };

    const sonar::SetHandler handler = SET_HANDLERS.value(target, defaultHandler);
    handler(message.value(PAYLOAD));
}

void handleRegister(const QVariantMap& message, QLocalSocket& from)
{
    const QString target(message.value(TARGET).toString());

    const sonar::RegisterHandler defaultHandler = [target](const QVariant&, QLocalSocket&, sonar::Notifier) {
        qWarning() << "No register handler for" << target;
    };

    const sonar::Notifier defaultNotifier = [target](QLocalSocket&, const QVariant&) {
        qWarning() << "No notifier for" << target;
    };

    const sonar::RegisterHandler handler = REGISTER_HANDLERS.value(target, defaultHandler);
    const sonar::Notifier publisher = NOTIFIERS.value(target, defaultNotifier);
    handler(message.value(PAYLOAD), from, publisher);
}

void handleUnregister(const QVariantMap& message, QLocalSocket& from)
{
    const QString target(message.value(TARGET).toString());

    const sonar::UnregisterHandler defaultHandler = [target](QLocalSocket&) {
        qWarning() << "No unregister handler for" << target;
    };

    const sonar::UnregisterHandler handler = UNREGISTER_HANDLERS.value(target, defaultHandler);
    handler(from);
}

void handleCommand(const QVariantMap& message)
{
    const QString target(message.value(TARGET).toString());

    const sonar::CommandHandler defaultHandler = [target](const QVariant&) {
        qWarning() << "No command handler for" << target;
    };

    const sonar::CommandHandler handler = COMMAND_HANDLERS.value(target, defaultHandler);
    handler(message.value(PAYLOAD));
}

} // namespace

namespace sonar {
namespace client {

void receive(const QVariantMap& message, QLocalSocket& from)
{
    const QString method(message.value(METHOD).toString());

    if(method == GET) {
        handleGet(message, from);
    }
    else if(method == SET) {
        handleSet(message);
    }
    else if(method == REGISTER) {
        handleRegister(message, from);
    }
    else if(method == UNREGISTER) {
        handleUnregister(message, from);
    }
    else if(method == COMMAND) {
        handleCommand(message);
    }
    else {
        qWarning() << "Unknown method" << method;
    }
}

void send(const QVariantMap& message, QLocalSocket& to)
{
    const QByteArray buffer(QJsonDocument::fromVariant(message).toJson(QJsonDocument::Compact));
    const qint64 bufferSize(buffer.size());

    to.write(reinterpret_cast<const char*>(&bufferSize), static_cast<qint64>(sizeof(bufferSize)));
    to.write(buffer);
    to.flush();
}

} // namespace client
} // namespace sonar
