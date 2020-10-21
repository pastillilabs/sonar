#include "calendar.h"

#include <QJsonObject>
#include <QJsonValue>
#include <mkcal-qt5/extendedcalendar.h>
#include <mkcal-qt5/extendedstorage.h>
#include <mkcal-qt5/extendedstorageobserver.h>

namespace {

QHash<JsonClient*, QMetaObject::Connection> clients;

/**
 * @brief The Calendar class
 */
class Calendar final
    : public mKCal::ExtendedStorageObserver
{

public:
    Calendar()
        : mCalendar(new mKCal::ExtendedCalendar(KDateTime::Spec::LocalZone()))
        , mStorage(mKCal::ExtendedCalendar::defaultStorage(mCalendar))
    {
        mStorage->open();
        mStorage->registerObserver(this);
    }

    ~Calendar() override
    {
        mStorage->unregisterObserver(this);
        mStorage->close();
        mCalendar->close();
    }

public: // From mKCal::ExtendedStorageObserver
    void storageModified(mKCal::ExtendedStorage* /*storage*/, const QString& /*info*/) override
    {
        const QJsonObject message{
            { QLatin1String("method"), QLatin1String("notify") },
            { QLatin1String("target"), QLatin1String("calendarChange") },
            { QLatin1String("payload"), QJsonValue() }
        };

        // Call all registered clients
        const auto constClients = clients.keys();
        for(JsonClient* client : constClients) {
            client->send(message);
        }
    }

    void storageProgress(mKCal::ExtendedStorage* /*storage*/, const QString& /*info*/) override
    {
    }

    void storageFinished(mKCal::ExtendedStorage* /*storage*/, bool /*error*/, const QString& /*info*/) override
    {
    }

private:
    mKCal::ExtendedCalendar::Ptr mCalendar;
    mKCal::ExtendedStorage::Ptr mStorage;
};

QScopedPointer<Calendar> calendar;

} // namespace

namespace sonar {
namespace observers {
namespace calendar {

void registerChangeObserver(JsonClient& jsonClient, const QJsonValue& /*payload*/)
{
    if(clients.isEmpty()) {
        ::calendar.reset(new Calendar());
    }

    if(!clients.contains(&jsonClient)) {
        QMetaObject::Connection connection = QObject::connect(&jsonClient, &JsonClient::destroyed, [&jsonClient] {
            unregisterChangeObserver(jsonClient);
        });

        clients.insert(&jsonClient, connection);
    }
}

void unregisterChangeObserver(JsonClient& jsonClient)
{
    if(clients.contains(&jsonClient)) {
        QObject::disconnect(clients.take(&jsonClient));
        clients.remove(&jsonClient);
    }

    if(clients.isEmpty()) {
        ::calendar.reset();
    }
}

} // namespace calendar
} // namespace observers
} // namespace sonar
