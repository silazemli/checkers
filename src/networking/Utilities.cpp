#include "networking/Utilities.hpp"

std::optional<Message> tryReadMessage(QIODevice& device, quint32& expectedSize) {
    if (expectedSize == 0) {
        if (device.bytesAvailable() < qint32(sizeof(quint32)))
            return std::nullopt;

        QDataStream in(&device);
        in >> expectedSize;
    }

    if (device.bytesAvailable() < expectedSize)
        return std::nullopt;

    QByteArray block = device.read(expectedSize);
    expectedSize = 0;

    QDataStream in(&block, QIODevice::ReadOnly);
    
    Message message;
    in >> message.action;

    message.payload = block.mid(in.device()->pos());
    return message;
}

void sendFrame(QIODevice& device, NetworkAction action, const QByteArray& payload) {
    QByteArray body;
    QDataStream bodyOut(&body, QIODevice::WriteOnly);
    
    bodyOut << action;
    bodyOut.writeRawData(payload.constData(), payload.size());

    QByteArray frame;
    QDataStream frameOut(&frame, QIODevice::WriteOnly);
    
    frameOut << quint32(body.size());
    frameOut.writeRawData(body.constData(), body.size());

    device.write(frame);
}

quint16 validatePort(const QString &string) {
    bool ok = false;

    quint16 port = string.toUShort(&ok);

    if (!ok || port == 0)
        throw std::invalid_argument("Port must be between 1 and 65535");

    return port;
}