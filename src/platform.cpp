#include "platform.h"

#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

namespace {

const QString SERVICE_ORG_BLUEZ(QStringLiteral("org.bluez"));
const QString SERVICE_NET_CONNMAN(QStringLiteral("net.connman"));
const QString SERVICE_ORG_OFONO(QStringLiteral("org.ofono"));
const QString SERVICE_COM_NOKIA_MCE(QStringLiteral("com.nokia.mce"));

const QString PATH_ORG_BLUEZ_HCI0(QStringLiteral("/org/bluez/hci0"));
const QString PATH_NET_CONNMAN_TECHNOLOGY_BLUETOOTH(QStringLiteral("/net/connman/technology/bluetooth"));
const QString PATH_NET_CONNMAN_TECHNOLOGY_WIFI(QStringLiteral("/net/connman/technology/wifi"));
const QString PATH_NET_CONNMAN_TECHNOLOGY_CELLULAR(QStringLiteral("/net/connman/technology/cellular"));
const QString PATH_RIL_0(QStringLiteral("/ril_0"));
const QString PATH_COM_NOKIA_MCE_REQUEST(QStringLiteral("/com/nokia/mce/request"));

const QString INTERFACE_ORG_FREEDESKTOP_DBUS_PROPERTIES(QStringLiteral("org.freedesktop.DBus.Properties"));
const QString INTERFACE_NET_CONNMAN_TECHNOLOGY(QStringLiteral("net.connman.Technology"));
const QString INTERFACE_ORG_OFONO_RADIOSETTINGS(QStringLiteral("org.ofono.RadioSettings"));
const QString INTERFACE_COM_NOKIA_MCE_REQUEST(QStringLiteral("com.nokia.mce.request"));

const QString METHOD_GET(QStringLiteral("Get"));
const QString METHOD_SET(QStringLiteral("Set"));
const QString METHOD_SETPROPERTY(QStringLiteral("SetProperty"));
const QString METHOD_REQ_RADIO_STATES_CHANGE(QStringLiteral("req_radio_states_change"));

const QString ARG_POWERED(QStringLiteral("Powered"));
const QString ARG_ORG_BLUEZ_ADAPTER1(QStringLiteral("org.bluez.Adapter1"));
const QString ARG_TECHNOLOGYPREFERENCE(QStringLiteral("TechnologyPreference"));

} // namespace

namespace sonar {
namespace platform {

void setBluetoothState(const QVariant& value)
{
    // TODO: Any better way to know which interface to use??
    QDBusInterface dbusInterface(SERVICE_ORG_BLUEZ,
                                 PATH_ORG_BLUEZ_HCI0,
                                 INTERFACE_ORG_FREEDESKTOP_DBUS_PROPERTIES,
                                 QDBusConnection::systemBus());
    const QDBusReply<QVariant> bluez5Reply(dbusInterface.call(METHOD_GET,
                                                              ARG_ORG_BLUEZ_ADAPTER1,
                                                              ARG_POWERED));
    if(bluez5Reply.isValid()) {
        dbusInterface.call(METHOD_SET,
                           ARG_ORG_BLUEZ_ADAPTER1,
                           ARG_POWERED,
                           QVariant::fromValue(QDBusVariant(value)));
    }
    else {
        QDBusInterface dbusInterfaceOld(SERVICE_NET_CONNMAN,
                                        PATH_NET_CONNMAN_TECHNOLOGY_BLUETOOTH,
                                        INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                        QDBusConnection::systemBus());
        dbusInterfaceOld.call(METHOD_SETPROPERTY,
                              ARG_POWERED,
                              QVariant::fromValue(QDBusVariant(value)));
    }
}

void setWifiState(const QVariant& value)
{
    QDBusInterface dbusInterface(SERVICE_NET_CONNMAN,
                                 PATH_NET_CONNMAN_TECHNOLOGY_WIFI,
                                 INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_POWERED,
                       QVariant::fromValue(QDBusVariant(value)));
}

void setCellularState(const QVariant& value)
{
    QDBusInterface dbusInterface(SERVICE_NET_CONNMAN,
                                 PATH_NET_CONNMAN_TECHNOLOGY_CELLULAR,
                                 INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_POWERED,
                       QVariant::fromValue(QDBusVariant(value)));
}

void setCellularRadioTechnology(const QVariant& value)
{
    QDBusInterface dbusInterface(SERVICE_ORG_OFONO,
                                 PATH_RIL_0,
                                 INTERFACE_ORG_OFONO_RADIOSETTINGS,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_TECHNOLOGYPREFERENCE,
                       QVariant::fromValue(QDBusVariant(value)));
}

void setFlightmodeState(const QVariant& value)
{
    QDBusInterface dbusInterface(SERVICE_COM_NOKIA_MCE,
                                 PATH_COM_NOKIA_MCE_REQUEST,
                                 INTERFACE_COM_NOKIA_MCE_REQUEST,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_REQ_RADIO_STATES_CHANGE,
                       static_cast<quint32>(value.toBool() ? 0 : 1),
                       static_cast<quint32>(1));
}

} // namespace platform
} // namespace sonar
