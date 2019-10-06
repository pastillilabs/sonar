#include "platform.h"
#include "observers/calendar.h"

#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <mkcal-qt5/extendedcalendar.h>
#include <mkcal-qt5/extendedstorage.h>
#include <mkcal-qt5/extendedstorageobserver.h>

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

const QString START_DATE(QStringLiteral("startDate"));
const QString END_DATE(QStringLiteral("endDate"));

} // namespace

namespace sonar {
namespace platform {

QVariant getCalendars(const QVariant& /*payload*/)
{
    mKCal::ExtendedCalendar::Ptr calendar(new mKCal::ExtendedCalendar(KDateTime::Spec::LocalZone()));
    mKCal::ExtendedStorage::Ptr storage(mKCal::ExtendedCalendar::defaultStorage(calendar));
    storage->open();

    QVariantList calendars;
    const mKCal::Notebook::List notebooks = storage->notebooks();
    for(const mKCal::Notebook::Ptr& notebook : notebooks) {
        const QVariantMap calendar{
            { QStringLiteral("name"), notebook->name() },
            { QStringLiteral("uid"), notebook->uid() }
        };
        calendars.append(calendar);
    }

    storage->close();
    calendar->close();

    return calendars;
}

QVariant getCalendarEvents(const QVariant& payload)
{
    const QVariantMap request(payload.toMap());
    const QDate startDate(QDate::fromString(request.value(START_DATE, QDate::currentDate().addDays(-1)).toString(), Qt::ISODate));
    const QDate endDate(QDate::fromString(request.value(END_DATE, QDate::currentDate().addDays(1)).toString(), Qt::ISODate));

    mKCal::ExtendedCalendar::Ptr calendar(new mKCal::ExtendedCalendar(KDateTime::Spec::LocalZone()));
    mKCal::ExtendedStorage::Ptr storage(mKCal::ExtendedCalendar::defaultStorage(calendar));
    storage->open();
    storage->load(startDate, endDate);
    storage->loadRecurringIncidences();

    KCalCore::Incidence::List incidences = calendar->incidences(startDate, endDate);
    const mKCal::ExtendedCalendar::ExpandedIncidenceList incidenceList = calendar->expandRecurrences(&incidences, KDateTime(startDate), KDateTime(endDate));

    QVariantList calendarEvents;
    for(const mKCal::ExtendedCalendar::ExpandedIncidence& expandedIncident : incidenceList) {
        const mKCal::ExtendedCalendar::ExpandedIncidenceValidity& incidenceValidity = expandedIncident.first;
        KCalCore::Incidence::Ptr incidence = expandedIncident.second;

        if(incidence->type() == KCalCore::IncidenceBase::TypeEvent) {
            const QString calendarUid(calendar->notebook(incidence));
            mKCal::Notebook::Ptr notebook = storage->notebook(calendarUid);

            if(!notebook || !notebook->isVisible() || !notebook->eventsAllowed()) {
                continue;
            }

            const QVariantMap event{
                { QStringLiteral("calendarId"), calendarUid },
                { QStringLiteral("begin"), QString::number(incidenceValidity.dtStart.toMSecsSinceEpoch()) },
                { QStringLiteral("end"), QString::number(incidenceValidity.dtEnd.toMSecsSinceEpoch()) },
                { QStringLiteral("title"), incidence->summary() },
                { QStringLiteral("description"), incidence->description() },
                { QStringLiteral("eventLocation"), incidence->location() },
                { QStringLiteral("hasAlarm"), incidence->hasEnabledAlarms() },
                { QStringLiteral("rrule"), incidence->recurs() ? QStringLiteral("RRULE") : QString() },
                { QStringLiteral("allDay"), incidence->allDay() },
                { QStringLiteral("availability"), static_cast<int>(incidence->status()) }
            };
            calendarEvents.append(event);
        }
    }

    storage->close();
    calendar->close();

    return calendarEvents;
}

QVariant getSupportedPermissions(const QVariant& /*payload*/)
{
    return QVariantList{
        QStringLiteral("sonar.permission.BLUETOOTH_STATE"),
        QStringLiteral("sonar.permission.CALENDAR"),
        QStringLiteral("sonar.permission.FLIGHTMODE_STATE"),
        QStringLiteral("sonar.permission.MOBILE_NETWORK_STATE"),
        QStringLiteral("sonar.permission.WIFI_STATE"),
    };
}

void registerCalendarChangeObserver(const QVariant& payload, QLocalSocket& client, Notifier notifier)
{
    observers::calendar::registerChangeObserver(payload, client, notifier);
}

void unregisterCalendarChangeObserver(QLocalSocket& client)
{
    observers::calendar::unregisterChangeObserver(client);
}

void setBluetoothState(const QVariant& payload)
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
                           QVariant::fromValue(QDBusVariant(payload)));
    }
    else {
        QDBusInterface dbusInterfaceOld(SERVICE_NET_CONNMAN,
                                        PATH_NET_CONNMAN_TECHNOLOGY_BLUETOOTH,
                                        INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                        QDBusConnection::systemBus());
        dbusInterfaceOld.call(METHOD_SETPROPERTY,
                              ARG_POWERED,
                              QVariant::fromValue(QDBusVariant(payload)));
    }
}

void setWifiState(const QVariant& payload)
{
    QDBusInterface dbusInterface(SERVICE_NET_CONNMAN,
                                 PATH_NET_CONNMAN_TECHNOLOGY_WIFI,
                                 INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_POWERED,
                       QVariant::fromValue(QDBusVariant(payload)));
}

void setCellularState(const QVariant& payload)
{
    QDBusInterface dbusInterface(SERVICE_NET_CONNMAN,
                                 PATH_NET_CONNMAN_TECHNOLOGY_CELLULAR,
                                 INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_POWERED,
                       QVariant::fromValue(QDBusVariant(payload)));
}

void setCellularRadioTechnology(const QVariant& payload)
{
    QDBusInterface dbusInterface(SERVICE_ORG_OFONO,
                                 PATH_RIL_0,
                                 INTERFACE_ORG_OFONO_RADIOSETTINGS,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_TECHNOLOGYPREFERENCE,
                       QVariant::fromValue(QDBusVariant(payload)));
}

void setFlightmodeState(const QVariant& payload)
{
    QDBusInterface dbusInterface(SERVICE_COM_NOKIA_MCE,
                                 PATH_COM_NOKIA_MCE_REQUEST,
                                 INTERFACE_COM_NOKIA_MCE_REQUEST,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_REQ_RADIO_STATES_CHANGE,
                       static_cast<quint32>(payload.toBool() ? 0 : 1),
                       static_cast<quint32>(1));
}

} // namespace platform
} // namespace sonar
