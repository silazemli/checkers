#pragma once

#include <iostream>
#include <algorithm>
#include <optional>

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QDebug>

#include "networking/Actions.hpp"
#include "networking/Utilities.hpp"
#include "game/Checkers.hpp"

class Server : public QObject {
    Q_OBJECT

private:
    struct Player {
        QTcpSocket* socket = nullptr;
        quint32 expectedMessageSize = 0;
        std::optional<game::Color> color;
    };

public:
    explicit Server(quint16 port, QObject* parent = nullptr);

    quint16 port() const { return server.serverPort(); }

private slots:
    void handleNewConnection();
    void handleDataReady();
    void handleRequestHighlighting(Player& player, QDataStream& in);
    void handleSubmitMove(Player& player, QDataStream& in);

private:
    void dispatch(Player& player, const Message& message);

    void sendAssignColor(Player& player, game::Color color);

    QTcpServer server;

    Player white;
    Player black;

    std::unique_ptr<game::Checkers> game;
};