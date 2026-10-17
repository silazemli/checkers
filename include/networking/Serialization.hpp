#pragma once

#include <vector>
#include <optional>

#include <QDataStream>

template<typename typeName>
inline QDataStream& operator<<(QDataStream& out, const std::optional<typeName>& optional) {
    bool hasValue = optional.has_value();
    
    out << hasValue;

    if (hasValue) out << *optional;

    return out;
}

template<typename typeName>
inline QDataStream& operator>>(QDataStream& in, std::optional<typeName>& optional) {
    bool hasValue;

    in >> hasValue;
    
    if (hasValue) {
        typeName value;
        in >> value;
        optional = value;
    } else
        optional = std::nullopt;

    return in;
}

template<typename typeName>
inline QDataStream& operator<<(QDataStream& out, const std::vector<typeName>& vector) {
    out << static_cast<quint32>(vector.size());

    for (const auto& item : vector) out << item;

    return out;
}

template<typename typeName>
inline QDataStream& operator>>(QDataStream& in, std::vector<typeName>& vector) {
    quint32 size;

    in >> size;

    vector.resize(size);

    for (quint32 i = 0; i < size; ++i) in >> vector[i];
    
    return in;
}