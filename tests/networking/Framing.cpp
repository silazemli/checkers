#include <cstdint>
#include <optional>

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QtTest>

#include "networking/Actions.hpp"
#include "networking/Message.hpp"
#include "networking/Utilities.hpp"

namespace {

QByteArray arguments(quint8 value) {
    QByteArray bytes;

    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_6);
    out << value;

    return bytes;
}

QByteArray frame(NetworkAction action, const QByteArray& arguments) {
    QByteArray wire;
    QBuffer device(&wire);

    device.open(QIODevice::WriteOnly);
    sendFrame(device, action, arguments);
    device.close();

    return wire;
}

} // namespace

class RoundTrip : public QObject {
    Q_OBJECT

private slots:
    void emptyPayload();
    void smallPayload();
    void largePayload();
};

void RoundTrip::emptyPayload() {
    QByteArray wire = frame(NetworkAction::AssignColor, {});
    QBuffer device(&wire);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(message.has_value());
    QCOMPARE(message->action, NetworkAction::AssignColor);
    QVERIFY(message->payload.isEmpty());
    QCOMPARE(expectedSize, 0u);
}

void RoundTrip::smallPayload() {
    const QByteArray arguments = arguments(7);

    QByteArray wire = frame(NetworkAction::AssignColor, arguments);
    QBuffer device(&wire);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(message.has_value());
    QCOMPARE(message->action, NetworkAction::AssignColor);
    QCOMPARE(message->payload, arguments);
    QCOMPARE(expectedSize, 0u);
}

void RoundTrip::largePayload() {
    QByteArray arguments(1024, '\0');

    for (qsizetype i = 0; i < arguments.size(); ++i)
        arguments[i] = static_cast<char>(i & 0xFF);

    QByteArray wire = frame(NetworkAction::UpdateBoardState, arguments);
    QBuffer device(&wire);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(message.has_value());
    QCOMPARE(message->action, NetworkAction::UpdateBoardState);
    QCOMPARE(message->payload, arguments);
    QCOMPARE(expectedSize, 0u);
}

class Partial : public QObject {
    Q_OBJECT

private slots:
    void lessThanPrefix();
    void prefixOnly();
    void prefixAndPartOfBody();
};

void Partial::lessThanPrefix() {
    QByteArray wire = frame(NetworkAction::AssignColor, arguments(1));
    wire.truncate(3);

    QBuffer device(&wire);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(!message.has_value());
    QCOMPARE(expectedSize, 0u);
}

void Partial::prefixOnly() {
    QByteArray wire = frame(NetworkAction::AssignColor, arguments(1));
    wire.truncate(sizeof(quint32));

    QBuffer device(&wire);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(!message.has_value());
    QVERIFY(expectedSize > 0u);
}

void Partial::prefixAndPartOfBody() {
    QByteArray wire = frame(NetworkAction::AssignColor, arguments(1));
    wire.truncate(sizeof(quint32) + 1);

    QBuffer device(&wire);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(!message.has_value());
    QVERIFY(expectedSize > 0u);
}

class Resume : public QObject {
    Q_OBJECT

private slots:
    void afterPrefixOnly();
    void afterPartOfBody();
};

void Resume::afterPrefixOnly() {
    const QByteArray arguments = arguments(42);
    const QByteArray wire = frame(NetworkAction::AssignColor, arguments);

    QByteArray received = wire.left(sizeof(quint32));
    QBuffer device(&received);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    QVERIFY(!tryReadMessage(device, expectedSize).has_value());
    QVERIFY(expectedSize > 0u);

    received.append(wire.mid(sizeof(quint32)));

    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(message.has_value());
    QCOMPARE(message->action, NetworkAction::AssignColor);
    QCOMPARE(message->payload, arguments);
    QCOMPARE(expectedSize, 0u);
}

void Resume::afterPartOfBody() {
    const QByteArray arguments = arguments(123);
    const QByteArray wire = frame(NetworkAction::AssignColor, arguments);

    const qsizetype split = sizeof(quint32) + (arguments.size() + 1) / 2;

    QByteArray received = wire.left(split);
    QBuffer device(&received);
    device.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;
    QVERIFY(!tryReadMessage(device, expectedSize).has_value());
    QVERIFY(expectedSize > 0u);

    received.append(wire.mid(split));

    const auto message = tryReadMessage(device, expectedSize);

    QVERIFY(message.has_value());
    QCOMPARE(message->action, NetworkAction::AssignColor);
    QCOMPARE(message->payload, arguments);
    QCOMPARE(expectedSize, 0u);
}

class Multiple : public QObject {
    Q_OBJECT

private slots:
    void twoFrames();
    void firstCompleteSecondPartial();
};

void Multiple::twoFrames() {
    const QByteArray first = arguments(1);
    const QByteArray second = arguments(2);

    QByteArray wire;
    {
        QBuffer device(&wire);
        device.open(QIODevice::WriteOnly);
        sendFrame(device, NetworkAction::AssignColor, first);
        sendFrame(device, NetworkAction::AssignColor, second);
    }

    QBuffer reader(&wire);
    reader.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;

    const auto message1 = tryReadMessage(reader, expectedSize);
    const auto message2 = tryReadMessage(reader, expectedSize);
    const auto message3 = tryReadMessage(reader, expectedSize);

    QVERIFY(message1.has_value());
    QVERIFY(message2.has_value());
    QVERIFY(!message3.has_value());

    QCOMPARE(message1->payload, first);
    QCOMPARE(message2->payload, second);
    QCOMPARE(expectedSize, 0u);
}

void Multiple::firstCompleteSecondPartial() {
    const QByteArray first = arguments(1);
    const QByteArray second = arguments(2);

    QByteArray wire;
    {
        QBuffer device(&wire);
        device.open(QIODevice::WriteOnly);
        sendFrame(device, NetworkAction::AssignColor, first);
        sendFrame(device, NetworkAction::AssignColor, second);
    }

    const qsizetype firstFrameLength = frame(NetworkAction::AssignColor, first).size();
    wire.truncate(firstFrameLength + 3);

    QBuffer reader(&wire);
    reader.open(QIODevice::ReadOnly);

    quint32 expectedSize = 0;

    const auto message1 = tryReadMessage(reader, expectedSize);
    const auto message2 = tryReadMessage(reader, expectedSize);

    QVERIFY(message1.has_value());
    QVERIFY(!message2.has_value());

    QCOMPARE(message1->payload, first);
    QCOMPARE(expectedSize, 0u);
}

std::int32_t main(std::int32_t argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    std::int32_t status = 0;
    auto run = [&](QObject* test) { status |= QTest::qExec(test, argc, argv); };

    RoundTrip roundTrip; run(&roundTrip);
    Partial partial; run(&partial);
    Resume resume; run(&resume);
    Multiple multiple; run(&multiple);

    return status;
}

#include "Framing.moc"