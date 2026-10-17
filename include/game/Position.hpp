#pragma once

#include <cstdint>

#include <QDataStream>

namespace game {

struct Position {
    std::int32_t row;
    std::int32_t column;
    
    bool operator==(const Position&) const = default;
};

inline QDataStream& operator<<(QDataStream& out, const Position& position) {
    out << position.row << position.column;
    return out;
}

inline QDataStream& operator>>(QDataStream& in, Position& position) {
    in >> position.row >> position.column;
    return in;
}

} // namespace