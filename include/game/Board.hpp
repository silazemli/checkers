#pragma once

#include <vector>
#include <optional>

#include "game/Piece.hpp"
#include "networking/Serialization.hpp"

namespace game {
using Board = std::vector<std::vector<std::optional<Piece>>>;
} // namespace