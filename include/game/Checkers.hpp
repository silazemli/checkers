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
    Board board;
    std::int32_t boardSize;
    std::int32_t numRows;
    
    Color currentPlayer;
    std::optional<Position> capturing;

    std::int32_t quietMoves;
    std::int32_t patience;

    static constexpr std::array<Position, 4> directions = {{
        {-1, -1}, {-1,  1}, { 1, -1}, { 1,  1}
    }};

public:
    Checkers(std::int32_t boardSize = 8, std::int32_t numRows = 2, std::int32_t patience = 40);

    inline const Board& getBoard() const { return board; };
    inline bool inBounds(std::int32_t row, std::int32_t column) const {
        return row >= 0 && row < boardSize &&
               column >= 0 && column < boardSize;
    }
    std::optional<Piece> at(std::int32_t row, std::int32_t column) const;
    std::optional<Piece> at(Position position) const;
    void place(Piece, Position);
    
    std::vector<Move> validMoves(Position position) const;
    std::vector<Move> captureMoves(Position position) const;
    std::vector<Move> simpleMoves(Position position) const;

    void perform(Position from, const Move& move);

    inline const Color getCurrentPlayer() const { return currentPlayer; }
    bool hasLegalMoves(Color player) const;

    std::optional<Position> getCapturing() const { return capturing; };

    GameState state() const;
};

} // namespace