#pragma once

#include <QDataStream>

namespace game {

enum struct Color : quint8 {
    Black, White
};

inline QDataStream& operator<<(QDataStream& out, Color color) {
    out << static_cast<quint8>(color);
    
    return out;
}

inline QDataStream& operator>>(QDataStream& in, Color& color) {
    quint8 value;
    in >> value;
    color = static_cast<Color>(value);
    
    return in;
}

} // namespace