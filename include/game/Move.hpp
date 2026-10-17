#pragma once

#include <optional>

#include <QDataStream>

#include "game/Position.hpp"
#include "networking/Serialization.hpp"

namespace game {

struct Move {
    Position to;
    std::optional<Position> captured;

    bool operator==(const Move&) const = default;
};

inline QDataStream& operator<<(QDataStream& outputStream, const Move& move) {
    outputStream << move.to << move.captured;

    return outputStream;
}

inline QDataStream& operator>>(QDataStream& inputStream, Move& move) {
    inputStream >> move.to >> move.captured;
    
    return inputStream;
}

} // namespace