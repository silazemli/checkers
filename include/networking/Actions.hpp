#pragma once

#include <QDataStream>

enum class NetworkAction : quint8 {
    RequestHighlighting,
    HighlightingResponse,
    SubmitMove,
    UpdateBoardState,
    AssignColor,
    EndGame
};

inline QDataStream& operator<<(QDataStream& outputStream, NetworkAction action) {
    outputStream << static_cast<quint8>(action);

    return outputStream;
}

inline QDataStream& operator>>(QDataStream& inputStream, NetworkAction& action) {
    quint8 value;
    inputStream >> value;
    action = static_cast<NetworkAction>(value);
    
    return inputStream;
}