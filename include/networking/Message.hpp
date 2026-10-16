#pragma once

#include <QByteArray>

#include "networking/Actions.hpp"

struct Message {
    NetworkAction action;
    QByteArray payload;
};