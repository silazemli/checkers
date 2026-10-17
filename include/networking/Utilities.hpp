#pragma once

#include <stdexcept>
#include <optional>

#include <QtTypes>
#include <QString>
#include <QByteArray>
#include <QIODevice>

#include "networking/Actions.hpp"
#include "networking/Message.hpp"

std::optional<Message> tryReadMessage(QIODevice& device, quint32& expectedSize);
void sendFrame(QIODevice& device, NetworkAction action, const QByteArray& payload);
template <typename... Arguments>
void sendMessage(QIODevice& device, NetworkAction action, const Arguments&... arguments) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);

    (out << ... << arguments);

    sendFrame(device, action, payload);
}


quint16 validatePort(const QString& string);
