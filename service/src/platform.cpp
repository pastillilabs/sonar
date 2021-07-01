#include "platform.h"
#include "observers/calendar.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QTimeZone>
#include <QtContacts/QContact>
#include <QtContacts/QContactDetail>
#include <QtContacts/QContactDisplayLabel>
#include <QtContacts/QContactManager>
#include <QtContacts/QContactName>
#include <QtContacts/QContactPhoneNumber>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <mkcal-qt5/extendedcalendar.h>
#include <mkcal-qt5/extendedstorage.h>
#include <mkcal-qt5/extendedstorageobserver.h>

namespace {

const QString CONTACT_MANAGER_NAME(QStringLiteral("org.nemomobile.contacts.sqlite"));

const QString SERVICE_COM_NOKIA_MCE(QStringLiteral("com.nokia.mce"));
const QString SERVICE_NET_CONNMAN(QStringLiteral("net.connman"));
const QString SERVICE_ORG_BLUEZ(QStringLiteral("org.bluez"));
const QString SERVICE_ORG_OFONO(QStringLiteral("org.ofono"));

const QString PATH_COM_NOKIA_MCE_REQUEST(QStringLiteral("/com/nokia/mce/request"));
const QString PATH_NET_CONNMAN_TECHNOLOGY_BLUETOOTH(QStringLiteral("/net/connman/technology/bluetooth"));
const QString PATH_NET_CONNMAN_TECHNOLOGY_CELLULAR(QStringLiteral("/net/connman/technology/cellular"));
const QString PATH_NET_CONNMAN_TECHNOLOGY_WIFI(QStringLiteral("/net/connman/technology/wifi"));
const QString PATH_ORG_BLUEZ_HCI0(QStringLiteral("/org/bluez/hci0"));
const QString PATH_RIL_0(QStringLiteral("/ril_0"));

const QString INTERFACE_COM_NOKIA_MCE_REQUEST(QStringLiteral("com.nokia.mce.request"));
const QString INTERFACE_NET_CONNMAN_TECHNOLOGY(QStringLiteral("net.connman.Technology"));
const QString INTERFACE_ORG_FREEDESKTOP_DBUS_PROPERTIES(QStringLiteral("org.freedesktop.DBus.Properties"));
const QString INTERFACE_ORG_OFONO_MESSAGEMANAGER(QStringLiteral("org.ofono.MessageManager"));
const QString INTERFACE_ORG_OFONO_RADIOSETTINGS(QStringLiteral("org.ofono.RadioSettings"));

const QString METHOD_GET(QStringLiteral("Get"));
const QString METHOD_REQ_RADIO_STATES_CHANGE(QStringLiteral("req_radio_states_change"));
const QString METHOD_SENDMESSAGE(QStringLiteral("SendMessage"));
const QString METHOD_SET(QStringLiteral("Set"));
const QString METHOD_SETPROPERTY(QStringLiteral("SetProperty"));

const QString ARG_ORG_BLUEZ_ADAPTER1(QStringLiteral("org.bluez.Adapter1"));
const QString ARG_POWERED(QStringLiteral("Powered"));
const QString ARG_TECHNOLOGYPREFERENCE(QStringLiteral("TechnologyPreference"));
const QString ARG_TETHERING(QStringLiteral("Tethering"));

const QString END_DATE(QStringLiteral("endDate"));
const QString START_DATE(QStringLiteral("startDate"));

} // namespace

namespace sonar {
namespace platform {

QJsonValue getCalendars(const QJsonValue& /*payload*/)
{
    mKCal::ExtendedCalendar::Ptr calendar(new mKCal::ExtendedCalendar(QTimeZone::systemTimeZone()));
    mKCal::ExtendedStorage::Ptr storage(mKCal::ExtendedCalendar::defaultStorage(calendar));
    storage->open();

    QJsonArray calendars;
    const mKCal::Notebook::List notebooks = storage->notebooks();
    for(const mKCal::Notebook::Ptr& notebook : notebooks) {
        const QJsonObject calendar{
            { QStringLiteral("name"), notebook->name() },
            { QStringLiteral("uid"), notebook->uid() }
        };
        calendars.append(calendar);
    }

    storage->close();
    calendar->close();

    return calendars;
}

QJsonValue getCalendarEvents(const QJsonValue& payload)
{
    const QJsonObject request(payload.toObject());
    const QDate startDate(QDate::fromString(request.value(START_DATE).toString(QDate::currentDate().addDays(-1).toString(Qt::ISODate)), Qt::ISODate));
    const QDate endDate(QDate::fromString(request.value(END_DATE).toString(QDate::currentDate().addDays(1).toString(Qt::ISODate)), Qt::ISODate));

    mKCal::ExtendedCalendar::Ptr calendar(new mKCal::ExtendedCalendar(QTimeZone::systemTimeZone()));
    mKCal::ExtendedStorage::Ptr storage(mKCal::ExtendedCalendar::defaultStorage(calendar));
    storage->open();
    storage->load(startDate, endDate);
    storage->loadRecurringIncidences();

    KCalendarCore::Incidence::List incidences(calendar->incidences(startDate, endDate));
    const mKCal::ExtendedCalendar::ExpandedIncidenceList incidenceList(calendar->expandRecurrences(&incidences, QDateTime(startDate), QDateTime(endDate)));

    QJsonArray calendarEvents;
    for(const mKCal::ExtendedCalendar::ExpandedIncidence& expandedIncident : incidenceList) {
        const mKCal::ExtendedCalendar::ExpandedIncidenceValidity& incidenceValidity(expandedIncident.first);
        const KCalendarCore::Incidence::Ptr incidence(expandedIncident.second);

        if(incidence->type() == KCalendarCore::IncidenceBase::TypeEvent) {
            const QString calendarUid(calendar->notebook(incidence));
            const mKCal::Notebook::Ptr notebook(storage->notebook(calendarUid));

            if(!notebook || !notebook->isVisible() || !notebook->eventsAllowed()) {
                continue;
            }

            const QJsonObject event{
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

QJsonValue getContacts(const QJsonValue& /*payload*/)
{
    QJsonArray contactList;

    const QtContacts::QContactManager contactManager(CONTACT_MANAGER_NAME);
    const QList<QtContacts::QContact> contacts(contactManager.contacts());

    for(const auto& contact : contacts) {
        QJsonArray numbers;
        const auto numberDetails(contact.details<QtContacts::QContactPhoneNumber>());
        for(const auto& numberDetail : numberDetails) {
            numbers.append(numberDetail.number());
        }

        const QJsonObject contactItem{
            { QStringLiteral("id"), contact.id().toString() },
            { QStringLiteral("name"), contact.detail<QtContacts::QContactDisplayLabel>().label() },
            { QStringLiteral("numbers"), numbers }
        };

        contactList.append(contactItem);
    }

    return contactList;
}

QJsonValue getSupportedPermissions(const QJsonValue& /*payload*/)
{
    return QJsonArray{
        QStringLiteral("sonar.permission.BLUETOOTH_STATE"),
        QStringLiteral("sonar.permission.CALENDAR"),
        QStringLiteral("sonar.permission.CONTACTS"),
        QStringLiteral("sonar.permission.FLIGHTMODE_STATE"),
        QStringLiteral("sonar.permission.MOBILE_NETWORK_STATE"),
        QStringLiteral("sonar.permission.SEND_SMS"),
        QStringLiteral("sonar.permission.WIFI_STATE"),
        QStringLiteral("sonar.permission.WIFI_TETHERING")
    };
}

void registerCalendarChangeObserver(JsonClient& jsonClient, const QJsonValue& payload)
{
    observers::calendar::registerChangeObserver(jsonClient, payload);
}

void unregisterCalendarChangeObserver(JsonClient& jsonClient)
{
    observers::calendar::unregisterChangeObserver(jsonClient);
}

void setBluetoothState(const QJsonValue& payload)
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
                           QVariant::fromValue(QDBusVariant(payload.toVariant())));
    }
    else {
        QDBusInterface dbusInterfaceOld(SERVICE_NET_CONNMAN,
                                        PATH_NET_CONNMAN_TECHNOLOGY_BLUETOOTH,
                                        INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                        QDBusConnection::systemBus());
        dbusInterfaceOld.call(METHOD_SETPROPERTY,
                              ARG_POWERED,
                              QVariant::fromValue(QDBusVariant(payload.toVariant())));
    }
}

void setWifiState(const QJsonValue& payload)
{
    QDBusInterface dbusInterface(SERVICE_NET_CONNMAN,
                                 PATH_NET_CONNMAN_TECHNOLOGY_WIFI,
                                 INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_POWERED,
                       QVariant::fromValue(QDBusVariant(payload.toVariant())));
}

void setCellularState(const QJsonValue& payload)
{
    QDBusInterface dbusInterface(SERVICE_NET_CONNMAN,
                                 PATH_NET_CONNMAN_TECHNOLOGY_CELLULAR,
                                 INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_POWERED,
                       QVariant::fromValue(QDBusVariant(payload.toVariant())));
}

void setCellularRadioTechnology(const QJsonValue& payload)
{
    QDBusInterface dbusInterface(SERVICE_ORG_OFONO,
                                 PATH_RIL_0,
                                 INTERFACE_ORG_OFONO_RADIOSETTINGS,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_TECHNOLOGYPREFERENCE,
                       QVariant::fromValue(QDBusVariant(payload.toVariant())));
}

void setFlightmodeState(const QJsonValue& payload)
{
    QDBusInterface dbusInterface(SERVICE_COM_NOKIA_MCE,
                                 PATH_COM_NOKIA_MCE_REQUEST,
                                 INTERFACE_COM_NOKIA_MCE_REQUEST,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_REQ_RADIO_STATES_CHANGE,
                       static_cast<quint32>(payload.toBool() ? 0 : 1),
                       static_cast<quint32>(1));
}

void setWifiTethering(const QJsonValue& payload)
{
    QDBusInterface dbusInterface(SERVICE_NET_CONNMAN,
                                 PATH_NET_CONNMAN_TECHNOLOGY_WIFI,
                                 INTERFACE_NET_CONNMAN_TECHNOLOGY,
                                 QDBusConnection::systemBus());
    dbusInterface.call(METHOD_SETPROPERTY,
                       ARG_TETHERING,
                       QVariant::fromValue(QDBusVariant(payload.toVariant())));
}

void commandSendSms(const QJsonValue& payload)
{
    const QJsonObject request(payload.toObject());
    const QString message{request.value(QStringLiteral("message")).toString()};
    const QString number{request.value(QStringLiteral("number")).toString()};

    QDBusInterface dbusInterface(SERVICE_ORG_OFONO,
                                 PATH_RIL_0,
                                 INTERFACE_ORG_OFONO_MESSAGEMANAGER,
                                 QDBusConnection::systemBus());

    dbusInterface.call(METHOD_SENDMESSAGE,
                       number,
                       message);
}

void commandUninstall(const QJsonValue& /*payload*/)
{
    QProcess::startDetached(QStringLiteral("rpm"),
                            QStringList{QStringLiteral("-e"),
                                        QStringLiteral("situations-sonar")});
}

} // namespace platform
} // namespace sonar
