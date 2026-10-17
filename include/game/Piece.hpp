#pragma once

#include <QDataStream>

#include "game/Color.hpp"

namespace game {

struct Piece {
    Color color;
    bool isKing;

    bool operator==(const Piece&) const = default;
    inline bool isOpponent(const Piece& other) const {
        return color != other.color;
    }
};

inline QDataStream& operator<<(QDataStream& out, const Piece& piece) {
    out << piece.color << piece.isKing;

    return out;
}

inline QDataStream& operator>>(QDataStream& in, Piece& piece) {
    in >> piece.color >> piece.isKing;
    
    return in;
}

} // namespace