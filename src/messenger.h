#ifndef MESSENGER_H
#define MESSENGER_H

#include <QLocalSocket>
#include <QVariantMap>

namespace sonar {

void receive(const QVariantMap& message, QLocalSocket& from);

} // namespace sonar

#endif // MESSENGER_H
