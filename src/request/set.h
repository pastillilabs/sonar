#ifndef REQUEST_SET_H
#define REQUEST_SET_H

#include <QLocalSocket>
#include <QVariantMap>

namespace sonar {
namespace request {

void set(const QVariantMap& message, QLocalSocket& from);

} // namespace request
} // namespace sonar

#endif // REQUEST_SET_H
