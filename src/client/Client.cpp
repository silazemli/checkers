#include "client/Client.hpp"

Client::Client(
    const QString& host,
    quint16 port,
    std::int32_t timeout,
    QObject* parent
) : QObject(parent) {
    socket.connectToHost(host, port);

    if (!socket.waitForConnected(timeout))
        throw std::runtime_error(socket.errorString().toStdString());
    
    connect(&socket, &QTcpSocket::readyRead, this, &Client::handleDataReady);

    if (socket.bytesAvailable() > 0)
        handleDataReady();
}

void Client::dispatch(const Message& message) {
    QDataStream in(message.payload);
    in.setVersion(QDataStream::Qt_6_6);

    switch (message.action) {
        case NetworkAction::AssignColor : {
            handleAssignColor(in);
            break;
        } case NetworkAction::HighlightingResponse : {
            handleHighlightingResponse(in);
            break;
        } default :
            qWarning() << "Unknown action:" << static_cast<int>(message.action);
            break;
    }

    if (in.status() != QDataStream::Ok) {
        qWarning() << "Malformed payload for action" << static_cast<int>(message.action);
        socket.abort();
    }
}

void Client::handleDataReady() {
    while (auto message = tryReadMessage(socket, expectedMessageSize))
        dispatch(*message);
}

void Client::handleAssignColor(QDataStream& in) {
    game::Color color;
    in >> color;

    playerColor = color;

    emit colorAssigned(color);
}

void Client::requestHighlighting(const game::Position& position) {
    sendMessage(socket, NetworkAction::RequestHighlighting, position);
}

void Client::handleHighlightingResponse(QDataStream& in) {
    std::vector<game::Position> cells;
    in >> cells;
    
    emit highlightingReceived(cells);
}