#pragma once

#include <QLocalSocket>
#include <QVariant>

#include <functional>

namespace sonar {

using Notifier = std::function<void(QLocalSocket&, const QVariant&)>;
using GetHandler = std::function<QVariant(const QVariant&)>;
using SetHandler = std::function<void(const QVariant&)>;
using RegisterHandler = std::function<void(const QVariant&, QLocalSocket&, Notifier)>;
using UnregisterHandler = std::function<void(QLocalSocket&)>;

} // namespace sonar
