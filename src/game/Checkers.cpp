#include "game/Checkers.hpp"

namespace game {

Checkers::Checkers(std::int32_t boardSize, std::int32_t numRows, std::int32_t patience)
    : boardSize(boardSize), numRows(numRows),
      boardState(boardSize, std::vector<std::optional<Piece>>(boardSize)),
      currentPlayer(Color::White), gameState(GameState::Ongoing),
      quietMoves(0), patience(patience) {
    for (std::int32_t row = 0; row < boardSize; ++row)
        for (std::int32_t column = 0; column < boardSize; ++column) {
            if ((row + column) % 2 == 0)
                continue;

            Color color;
            if (row < numRows)
                color = Color::Black;
            else if (row >= boardSize - numRows)
                color = Color::White;
            else 
                continue;

            boardState[row][column] = Piece{color, false};
        }
}

void Checkers::place(Piece piece, Position position) {
    if (!inBounds(position.row, position.column))
        throw std::out_of_range("Board position out of bounds");

    boardState[position.row][position.column] = piece;
}

std::optional<Piece> Checkers::at(Position position) const {
    return at(position.row, position.column);
}

std::optional<Piece> Checkers::at(std::int32_t row, std::int32_t column) const {
    if (!inBounds(row, column))
        throw std::out_of_range("Board position out of bounds");

    return boardState[row][column];
}

std::vector<Move> Checkers::validMoves(Position from) const {
    auto piece = at(from);
    if (!piece) return {};

    if (capturing && from != *capturing)
            return {};
    
    std::vector<Move> captures = captureMoves(from);

    if (!captures.empty())
        return captures;

    return simpleMoves(from);
}

std::vector<Move> Checkers::captureMoves(Position from) const {
    auto piece = at(from);
    if (!piece) return {};
    std::int32_t row = from.row;
    std::int32_t column = from.column;

    std::vector<Move> captures;

    for (auto& direction : directions) {
        std::int32_t directionRow = direction.row;
        std::int32_t directionColumn = direction.column;

        if (piece->isKing) {
            // a king scans until it finds the first opponent
            std::int32_t scanRow = row + directionRow;
            std::int32_t scanColumn = column + directionColumn;

            while (inBounds(scanRow, scanColumn) &&
                   !boardState[scanRow][scanColumn].has_value()) {
                scanRow += directionRow;
                scanColumn += directionColumn;
            }

            if (inBounds(scanRow, scanColumn) &&
                boardState[scanRow][scanColumn].has_value() &&
                piece->isOpponent(*boardState[scanRow][scanColumn])) {
                std::int32_t landingRow = scanRow + directionRow;
                std::int32_t landingColumn = scanColumn + directionColumn;
                // a king may land on any empty square beyond the captured piece
                while (inBounds(landingRow, landingColumn) &&
                       !boardState[landingRow][landingColumn].has_value()) {
                    captures.push_back(
                        Move{{landingRow, landingColumn}, Position{scanRow, scanColumn}});
                    landingRow += directionRow;
                    landingColumn += directionColumn;
                }
            }
        } else {
            // a man captures by jumping a single opponent piece
            std::int32_t jumpedRow = row + directionRow;
            std::int32_t jumpedColumn = column + directionColumn;

            if (!inBounds(jumpedRow, jumpedColumn) ||
                !boardState[jumpedRow][jumpedColumn].has_value() ||
                !piece->isOpponent(*boardState[jumpedRow][jumpedColumn]))
                continue;

            std::int32_t landingRow = jumpedRow + directionRow;
            std::int32_t landingColumn = jumpedColumn + directionColumn;

            if (inBounds(landingRow, landingColumn) &&
                !boardState[landingRow][landingColumn].has_value())
                captures.push_back(
                    Move{{landingRow, landingColumn}, Position{jumpedRow, jumpedColumn}});
        }
    }

    return captures;
}

std::vector<Move> Checkers::simpleMoves(Position from) const {
    auto piece = at(from);
    if (!piece) return {};
    std::int32_t row = from.row;
    std::int32_t column = from.column;

    std::vector<Move> moves;

    if (piece->isKing)
        // a king may move any distance along a clean diagonal
        for (auto& direction : directions) {
            std::int32_t directionRow = direction.row;
            std::int32_t directionColumn = direction.column;

            std::int32_t nextRow = row + directionRow;
            std::int32_t nextColumn = column + directionColumn;

            while (inBounds(nextRow, nextColumn) &&
                   !boardState[nextRow][nextColumn].has_value()) {
                moves.push_back({{nextRow, nextColumn}, std::nullopt});
                nextRow += directionRow;
                nextColumn += directionColumn;
            }
        }
    else {
        std::int32_t direction = piece->color == Color::Black ? 1 : -1;

        for (std::int32_t columnDirection : {-1, 1}) {
            std::int32_t nextRow = row + direction;
            std::int32_t nextColumn = column + columnDirection;

            if (inBounds(nextRow, nextColumn) &&
                !boardState[nextRow][nextColumn].has_value())
                moves.push_back({{nextRow, nextColumn}, std::nullopt});
        }
    }

    return moves;
}

void Checkers::perform(Position from, const Move& move) {
    // move assumed to be valid
    ++quietMoves;

    Position to = move.to;
    boardState[to.row][to.column] = boardState[from.row][from.column];

    boardState[from.row][from.column] = std::nullopt;

    if (move.captured.has_value()) {
        Position captured = *move.captured;
        boardState[captured.row][captured.column] = std::nullopt;
        quietMoves = 0;
    }

    auto& piece = boardState[to.row][to.column];
    if (piece && !piece->isKing)
        if ((piece->color == Color::Black && move.to.row == boardSize - 1) ||
            (piece->color == Color::White && move.to.row == 0)) {
            piece->isKing = true;
            quietMoves = 0;
        }

    if (move.captured.has_value() &&
        !captureMoves(to).empty())
        capturing = to;
    else {
        capturing.reset();
        currentPlayer = (currentPlayer == Color::Black) ? Color::White : Color::Black;
    }
}

bool Checkers::hasLegalMoves(Color player) const {
    for (std::int32_t row = 0; row < boardSize; ++row)
        for (std::int32_t column = 0; column < boardSize; ++column) {
            const auto& square = boardState[row][column];
            if (square &&
                square->color == player &&
                !validMoves({row, column}).empty())
                return true;
        }
    
    return false;
}

void Checkers::state() {
    if (quietMoves >= patience)
        return GameState::Draw;
    if (!hasLegalMoves(currentPlayer))
        return currentPlayer == Color::White ?
                                            GameState::BlackWins :
                                            GameState::WhiteWins;
    return GameState::Ongoing;
}

} // namespace