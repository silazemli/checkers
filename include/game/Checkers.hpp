#pragma once

#include <vector>
#include <array>
#include <optional>
#include <stdexcept>

#include "game/Board.hpp"
#include "game/Position.hpp"
#include "game/Move.hpp"
#include "game/GameState.hpp"
#include "game/Piece.hpp"

namespace game {

class Checkers {
private:
    std::int32_t boardSize;
    std::int32_t numRows;

    Board board;
    
    Color currentPlayer;
    std::optional<Position> capturing;

    std::int32_t quietMoves;
    std::int32_t patience;

    static constexpr std::array<Position, 4> directions = {
        Position{-1, -1}, 
        Position{-1,  1}, 
        Position{ 1, -1}, 
        Position{ 1,  1}
    };

public:
    Checkers(std::int32_t boardSize = 8, std::int32_t numRows = 2, std::int32_t patience = 40);

    bool inBounds(std::int32_t row, std::int32_t column) const;

    std::optional<Piece> at(std::int32_t row, std::int32_t column) const;
    std::optional<Piece> at(Position position) const;

    void place(Piece, Position);

    std::vector<Move> validMoves(Position position) const;
    std::vector<Move> captureMoves(Position position) const;
    std::vector<Move> simpleMoves(Position position) const;

    void perform(Position from, const Move& move);

    Color player() const;
    std::optional<Position> activePiece() const;

    bool hasLegalMoves(Color player) const;

    GameState check() const;
};

} // namespace