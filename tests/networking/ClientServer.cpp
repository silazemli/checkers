#include <cstdint>
#include <optional>
#include <vector>

#include <QtTest>

#include "client/Client.hpp"
#include "game/Color.hpp"
#include "game/Position.hpp"
#include "server/Server.hpp"

class Handshake : public QObject {
    Q_OBJECT

private slots:
    void assignsWhiteAndBlack();
};

void Handshake::assignsWhiteAndBlack() {
    Server server(0);
    const quint16 port = server.port();

    Client white("127.0.0.1", port);
    Client black("127.0.0.1", port);

    std::optional<game::Color> whiteColor;
    std::optional<game::Color> blackColor;

    connect(&white, &Client::colorAssigned,
        [&](game::Color color) { whiteColor = color; });
    connect(&black, &Client::colorAssigned,
        [&](game::Color color) { blackColor = color; });

    QTRY_VERIFY(whiteColor.has_value());
    QTRY_VERIFY(blackColor.has_value());

    QCOMPARE(*whiteColor, game::Color::WHITE);
    QCOMPARE(*blackColor, game::Color::BLACK);
}

class Highlighting : public QObject {
    Q_OBJECT

private slots:
    void roundTrip();
};

void Highlighting::roundTrip() {
    Server server(0);
    const quint16 port = server.port();

    Client white("127.0.0.1", port);
    Client black("127.0.0.1", port);

    bool whiteReady = false;
    connect(&white, &Client::colorAssigned,
        [&](game::Color) { whiteReady = true; });
    QTRY_VERIFY(whiteReady);

    bool received = false;
    std::vector<game::Position> cells;
    connect(&white, &Client::highlightingReceived,
        [&](const std::vector<game::Position>& color) {
            received = true;
            cells = color;
        });

    white.requestHighlighting(game::Position{0, 0});

    QTRY_VERIFY(received);
}

std::int32_t main(std::int32_t argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    std::int32_t status = 0;
    auto run = [&](QObject* test) { status |= QTest::qExec(test, argc, argv); };

    Handshake handshake; run(&handshake);
    Highlighting highlighting; run(&highlighting);

    return status;
}

#include "ClientServer.moc"