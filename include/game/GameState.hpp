#pragma once

#include <QDataStream>

namespace game {

enum struct GameState : quint8 {
    ONGOING,
    DRAW,
    WHITE_WINS,
    BLACK_WINS
};

inline QDataStream& operator<<(QDataStream& out, GameState state) {
    out << static_cast<quint8>(state);
    
    return out;
}

inline QDataStream& operator>>(QDataStream& in, GameState& state) {
    quint8 value;
    in >> value;
    state = static_cast<GameState>(value);

    return in;
}

} // namespace