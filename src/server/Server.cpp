#include "server/Server.hpp"

Server::Server(quint16 port, QObject* parent) : QObject(parent) {
    connect(&server, &QTcpServer::newConnection, this, &Server::handleNewConnection);

    if (!server.listen(QHostAddress::Any, port))
       throw std::runtime_error(server.errorString().toStdString());
}

void Server::handleNewConnection() {
    QTcpSocket* socket = server.nextPendingConnection();

    if (!white.socket) {
        white.socket = socket;
        connect(socket, &QTcpSocket::readyRead, this, &Server::handleDataReady);
        std::cout << "White connected" << std::endl;
        // connect(socket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
    } else if (!black.socket) {
        black.socket = socket;
        connect(socket, &QTcpSocket::readyRead, this, &Server::handleDataReady);
        std::cout << "Black connected" << std::endl;
        // connect(socket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
        
        sendAssignColor(white, game::Color::White);
        sendAssignColor(black, game::Color::Black);

        game = std::make_unique<game::Checkers>();
    } else
        socket->disconnectFromHost();
}

void Server::handleDataReady() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    Player* player = (white.socket == socket) ? &white
                : (black.socket == socket) ? &black
                : nullptr;
    if (!player) {
        qWarning() << "readyRead from unknown socket";
        return;
    }

    while (auto message = tryReadMessage(*socket, player->expectedMessageSize))
        dispatch(*player, *message);
}

void Server::dispatch(Player& player, const Message& message) {
    QDataStream in(message.payload);
    in.setVersion(QDataStream::Qt_6_6);

    switch (message.action) {
        case NetworkAction::SubmitMove: {
            handleSubmitMove(player, in);
            break;
        } case NetworkAction::RequestHighlighting: {
            handleRequestHighlighting(player, in);
            break;
        } default: {
            qWarning() << "Unknown action:" << static_cast<int>(message.action);
            break;
        }
    }

    if (in.status() != QDataStream::Ok) {
        qWarning() << "Malformed payload for action" << static_cast<int>(message.action);
        player.socket->abort();
    }
}

void Server::sendAssignColor(Player& player, game::Color color) {
    sendMessage(*player.socket, NetworkAction::AssignColor, color);
}

void Server::handleRequestHighlighting(Player& player, QDataStream& in) {
    game::Position position;
    in >> position;

    std::vector<game::Move> moves = game->validMoves(position);
    
    std::vector<game::Position> cells;
    cells.reserve(moves.size());
    for (const game::Move& move : moves)
        cells.push_back(move.to);

    sendMessage(*player.socket, NetworkAction::HighlightingResponse, cells);
}

void Server::handleSubmitMove(Player& player, QDataStream& in) {
    game::Position from;
    game::Position to;

    in >> from >> to;

    std::vector<game::Move> moves = game->validMoves(from);
    auto iterator = std::find_if(
        moves.begin(),
        moves.end(),
        [&to](game::Move move) {
            return move.to == to;
        });
    
    if (iterator == moves.end()) {
        qWarning() << "Illegal SubmitMove";
        return;
    }
    game::Move move = *iterator;

    game->perform(from, move);

    game::GameState gameState = game->state();
    if (gameState == game::GameState::Ongoing) {
        sendMessage(*white.socket, NetworkAction::UpdateBoardState, game->getBoard());
        sendMessage(*black.socket, NetworkAction::UpdateBoardState, game->getBoard());
    } else {
        sendMessage(*white.socket, NetworkAction::EndGame, gameState);
        sendMessage(*black.socket, NetworkAction::EndGame, gameState);
    }
}