#pragma once

#include <cstdint>
#include <optional>
#include <exception>
#include <iostream>

#include <QString>
#include <QObject>
#include <QTcpSocket>

#include "networking/Actions.hpp"
#include "networking/Utilities.hpp"
#include "game/Move.hpp"
#include "game/Color.hpp"
#include "game/Position.hpp"
#include "game/Board.hpp"

class Client : public QObject {
    Q_OBJECT

public:
    explicit Client(
        const QString& host,
        quint16 port,
        std::int32_t timeout = 3000,
        QObject* parent = nullptr
    );

    void requestHighlighting(const game::Position& position);
  
private slots:
    void handleDataReady();

signals:
    void colorAssigned(game::Color color);
    void highlightingReceived(std::vector<game::Position> cells);

private:
    QTcpSocket socket;
    quint32 expectedMessageSize = 0;

    std::optional<game::Color> playerColor;

    void dispatch(const Message& message);

    void handleAssignColor(QDataStream& in);
    void handleHighlightingResponse(QDataStream& in);
};