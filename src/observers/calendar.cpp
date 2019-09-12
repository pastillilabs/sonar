#include "calendar.h"

#include <mkcal-qt5/extendedcalendar.h>
#include <mkcal-qt5/extendedstorage.h>
#include <mkcal-qt5/extendedstorageobserver.h>

namespace {

QHash<QLocalSocket*, sonar::Notifier> notifiers;
QHash<QLocalSocket*, QMetaObject::Connection> connections;

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

    ~Calendar()
    {
        mStorage->unregisterObserver(this);
        mStorage->close();
        mCalendar->close();
    }

public: // From mKCal::ExtendedStorageObserver
    void storageModified(mKCal::ExtendedStorage* /*storage*/, const QString& /*info*/) override
    {
        // Call all notifiers
        auto i = notifiers.begin();
        while(i != notifiers.end()) {
            const auto& notifier = i.value();
            notifier(*i.key(), QVariant());
            ++i;
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

void registerChangeObserver(const QVariant& /*payload*/, QLocalSocket& client, Notifier notifier)
{
    if(notifiers.isEmpty()) {
        ::calendar.reset(new Calendar());
    }

    if(!notifiers.contains(&client)) {
        QMetaObject::Connection connection = QObject::connect(&client, &QLocalSocket::destroyed, [&client] {
            unregisterChangeObserver(client);
        });

        connections.insert(&client, connection);
        notifiers.insert(&client, notifier);
    }
}

void unregisterChangeObserver(QLocalSocket& client)
{
    if(notifiers.contains(&client)) {
        QObject::disconnect(connections.take(&client));
        notifiers.remove(&client);
    }

    if(notifiers.isEmpty()) {
        ::calendar.reset();
    }
}

} // namespace calendar
} // namespace observers
} // namespace sonar
