#include <array>
#include <algorithm>
#include <vector>
#include <utility>
#include <cstdint>

#include <QtTest>

#include "game/Checkers.hpp"

namespace {

static game::Checkers cleanBoard() {
    return game::Checkers(8, 0);
}

static void place(
    game::Checkers& checkers, std::int32_t row, std::int32_t column,
    game::Color color, bool isKing = false) {
    checkers.place(game::Piece{color, isKing}, game::Position{row, column});
}

static bool isPiece(
    const std::optional<game::Piece>& piece,
    game::Color color, bool isKing = false) {
    return piece.has_value() &&
            piece->color == color &&
            piece->isKing == isKing;
}

void compareMoves(
    const std::vector<game::Move>& actual,
    const std::vector<std::pair<std::int32_t, std::int32_t>>& expected
) {
    std::vector<std::pair<std::int32_t, std::int32_t>> actualPositions;
    actualPositions.reserve(actual.size());

    for (const game::Move& move : actual)
        actualPositions.emplace_back(move.to.row, move.to.column);

    std::sort(actualPositions.begin(), actualPositions.end());

    auto expectedSorted = expected;
    std::sort(expectedSorted.begin(), expectedSorted.end());

    QCOMPARE(actualPositions, expectedSorted);
}

void compareCaptures(
    const std::vector<game::Move>& actual,
    const std::vector<std::pair<std::int32_t, std::int32_t>>& expected
) {
    std::vector<std::pair<std::int32_t, std::int32_t>> actualCaptures;
    actualCaptures.reserve(actual.size());

    for (const game::Move& move : actual) {
        QVERIFY(move.captured.has_value());
        actualCaptures.emplace_back(move.captured->row, move.captured->column);
    }

    std::sort(actualCaptures.begin(), actualCaptures.end());

    auto expectedSorted = expected;
    std::sort(expectedSorted.begin(), expectedSorted.end());

    QCOMPARE(actualCaptures, expectedSorted);
}

}

class Board : public QObject {
    Q_OBJECT

private slots:
    void initialBoard();
    void outOfBounds();
};

void Board::initialBoard() {
    game::Checkers checkers(8, 3);

    const std::array<std::pair<std::int32_t,std::int32_t>, 12> blackPositions = {{
        {0, 1}, {0, 3}, {0, 5}, {0, 7},
        {1, 0}, {1, 2}, {1, 4}, {1, 6},
        {2, 1}, {2, 3}, {2, 5}, {2, 7}
    }};
    const std::array<std::pair<std::int32_t,std::int32_t>, 12> whitePositions = {{
        {5, 0}, {5, 2}, {5, 4}, {5, 6},
        {6, 1}, {6, 3}, {6, 5}, {6, 7},
        {7, 0}, {7, 2}, {7, 4}, {7, 6}
    }};

    for (auto [row, column] : blackPositions)
        QVERIFY(isPiece(checkers.at(row, column), game::Color::Black));

    for (auto [row, column] : whitePositions)
        QVERIFY(isPiece(checkers.at(row, column), game::Color::White));

    QVERIFY(!checkers.at(0, 0).has_value());
    QVERIFY(!checkers.at(3, 3).has_value());
    QVERIFY(!checkers.at(4, 4).has_value());
}

void Board::outOfBounds() {
    game::Checkers checkers;

    QVERIFY_THROWS_EXCEPTION(std::out_of_range, checkers.at(8, 0));
    QVERIFY_THROWS_EXCEPTION(std::out_of_range, checkers.at(0, 8));
    QVERIFY_THROWS_EXCEPTION(std::out_of_range, checkers.at(-1, 0));
    QVERIFY_THROWS_EXCEPTION(std::out_of_range, checkers.at(0, -1));
}
class SimpleMoves : public QObject {
    Q_OBJECT

private slots:
    void blackMan();
    void whiteMan();
    void king();
    void blocked();
};

void SimpleMoves::blackMan() {
    auto checkers = cleanBoard();
    place(checkers, 1, 2, game::Color::Black);

    compareMoves(checkers.simpleMoves({1, 2}), {{2, 1}, {2, 3}});
}

void SimpleMoves::whiteMan() {
    auto checkers = cleanBoard();
    place(checkers, 5, 2, game::Color::White);

    compareMoves(checkers.simpleMoves({5, 2}), {{4, 1}, {4, 3}});
}

void SimpleMoves::king() {
    auto checkers = cleanBoard();
    place(checkers, 4, 3, game::Color::Black, true);

    compareMoves(
        checkers.simpleMoves({4, 3}),
        {
            {3, 2}, {2, 1}, {1, 0},
            {3, 4}, {2, 5}, {1, 6}, {0, 7},
            {5, 2}, {6, 1}, {7, 0},
            {5, 4}, {6, 5}, {7, 6}
        }
    );
}

void SimpleMoves::blocked() {
    auto checkers = cleanBoard();

    place(checkers, 4, 3, game::Color::Black, true);
    place(checkers, 2, 1, game::Color::Black);
    place(checkers, 3, 4, game::Color::Black);

    compareMoves(
        checkers.simpleMoves({4, 3}),
        {
            {3, 2},
            {5, 2}, {6, 1}, {7, 0},
            {5, 4}, {6, 5}, {7, 6}
        }
    );
}

class CaptureMoves : public QObject {
    Q_OBJECT

private slots:
    void man();
    void manIgnoresOwn();
    void kingAdjacent();
    void kingFlying();
    void kingStops();
};

void CaptureMoves::man() {
    auto checkers = cleanBoard();

    place(checkers, 3, 2, game::Color::Black);
    place(checkers, 2, 1, game::Color::White);
    place(checkers, 2, 3, game::Color::White);
    place(checkers, 4, 1, game::Color::White);
    place(checkers, 4, 3, game::Color::White);

    auto moves = checkers.validMoves({3, 2});

    compareMoves(moves, {{1, 0}, {1, 4}, {5, 0}, {5, 4}});
    compareCaptures(moves, {{2, 1}, {2, 3}, {4, 1}, {4, 3}});
}

void CaptureMoves::manIgnoresOwn() {
    auto checkers = cleanBoard();

    place(checkers, 3, 2, game::Color::Black);
    place(checkers, 2, 1, game::Color::Black);

    QVERIFY(checkers.captureMoves({3, 2}).empty());
}

void CaptureMoves::kingAdjacent() {
    auto checkers = cleanBoard();

    place(checkers, 4, 3, game::Color::Black, true);
    place(checkers, 3, 2, game::Color::White);

    auto moves = checkers.validMoves({4, 3});

    compareMoves(moves, {{2, 1}, {1, 0}});
    compareCaptures(moves, {{3, 2}, {3, 2}});
}

void CaptureMoves::kingFlying() {
    auto checkers = cleanBoard();

    place(checkers, 7, 0, game::Color::Black, true);
    place(checkers, 4, 3, game::Color::White);

    auto moves = checkers.validMoves({7, 0});

    compareMoves(moves, {{3, 4}, {2, 5}, {1, 6}, {0, 7}});
    compareCaptures(moves, {{4, 3}, {4, 3}, {4, 3}, {4, 3}});
}

void CaptureMoves::kingStops() {
    auto checkers = cleanBoard();

    place(checkers, 7, 0, game::Color::Black, true);
    place(checkers, 4, 3, game::Color::White);
    place(checkers, 1, 6, game::Color::White);

    auto moves = checkers.validMoves({7, 0});

    compareMoves(moves, {{3, 4}, {2, 5}});
    compareCaptures(moves, {{4, 3}, {4, 3}});
}

class ValidMoves : public QObject {
    Q_OBJECT

private slots:
    void prefersCapture();
    void emptySquare();
    void fallsBackToSimple();
};

void ValidMoves::prefersCapture() {
    auto checkers = cleanBoard();

    place(checkers, 3, 2, game::Color::White);
    place(checkers, 2, 1, game::Color::Black);

    compareMoves(checkers.validMoves({3, 2}), {{1, 0}});
}

void ValidMoves::emptySquare() {
    auto checkers = cleanBoard();

    QVERIFY(checkers.validMoves({3, 2}).empty());
}

void ValidMoves::fallsBackToSimple() {
    auto checkers = cleanBoard();

    place(checkers, 3, 2, game::Color::White);

    compareMoves(checkers.validMoves({3, 2}), {{2, 1}, {2, 3}});
}

class Perform : public QObject {
    Q_OBJECT

private slots:
    void simpleMove();
    void capture();
    void promotion();
    void switchesTurn();
    void keepsTurnOnMultiCapture();
};

void Perform::simpleMove() {
    auto checkers = cleanBoard();
    place(checkers, 5, 2, game::Color::White);

    checkers.perform({5, 2}, {{4, 1}, std::nullopt});

    QVERIFY(!checkers.at(5, 2).has_value());
    QVERIFY(isPiece(checkers.at(4, 1), game::Color::White));
}

void Perform::capture() {
    auto checkers = cleanBoard();
    place(checkers, 5, 2, game::Color::White);
    place(checkers, 4, 1, game::Color::Black);

    checkers.perform({5, 2}, {{3, 0}, game::Position{4, 1}});

    QVERIFY(!checkers.at(5, 2).has_value());
    QVERIFY(!checkers.at(4, 1).has_value());
    QVERIFY(isPiece(checkers.at(3, 0), game::Color::White));
}

void Perform::promotion() {
    auto checkers = cleanBoard();
    place(checkers, 1, 0, game::Color::White);

    checkers.perform({1, 0}, {{0, 1}, std::nullopt});

    QVERIFY(!checkers.at(1, 0).has_value());
    QVERIFY(isPiece(checkers.at(0, 1), game::Color::White, true));
}

void Perform::switchesTurn() {
    auto checkers = cleanBoard();
    QCOMPARE(checkers.player(), game::Color::White);

    place(checkers, 5, 2, game::Color::White);
    checkers.perform({5, 2}, {{4, 1}, std::nullopt});

    QCOMPARE(checkers.player(), game::Color::Black);
    QVERIFY(!checkers.activePiece().has_value());
}

void Perform::keepsTurnOnMultiCapture() {
    auto checkers = cleanBoard();
    place(checkers, 5, 2, game::Color::White);
    place(checkers, 4, 1, game::Color::Black);
    place(checkers, 2, 1, game::Color::Black);

    checkers.perform({5, 2}, {{3, 0}, game::Position{4, 1}});

    QVERIFY(!checkers.at(4, 1).has_value());
    QVERIFY(isPiece(checkers.at(3, 0), game::Color::White));

    QCOMPARE(checkers.player(), game::Color::White);
    auto active = checkers.activePiece();
    QVERIFY(active.has_value());
    QCOMPARE(active->row, 3);
    QCOMPARE(active->column, 0);
}

class LegalMoves : public QObject {
    Q_OBJECT

private slots:
    void emptyBoard();
    void manWithOpenDiagonals();
    void ignoresOtherColor();
    void blockedByOwnPieces();
    void blockedByEnemyPieces();
    void king();
    void kingBlocked();
    void atEdge();
};

void LegalMoves::emptyBoard() {
    auto checkers = cleanBoard();
    QVERIFY(!checkers.hasLegalMoves(game::Color::White));
    QVERIFY(!checkers.hasLegalMoves(game::Color::Black));
}

void LegalMoves::manWithOpenDiagonals() {
    auto checkers = cleanBoard();
    place(checkers, 3, 2, game::Color::White);

    QVERIFY(checkers.hasLegalMoves(game::Color::White));
    QVERIFY(!checkers.hasLegalMoves(game::Color::Black));
}

void LegalMoves::ignoresOtherColor() {
    auto checkers = cleanBoard();
    place(checkers, 3, 2, game::Color::White);

    QVERIFY(checkers.hasLegalMoves(game::Color::White));
    QVERIFY(!checkers.hasLegalMoves(game::Color::Black));
}

void LegalMoves::blockedByOwnPieces() {
    auto checkers = cleanBoard();
    place(checkers, 1, 0, game::Color::White);
    place(checkers, 0, 1, game::Color::White);

    QVERIFY(!checkers.hasLegalMoves(game::Color::White));
}

void LegalMoves::blockedByEnemyPieces() {
    auto checkers = cleanBoard();
    place(checkers, 3, 2, game::Color::White);
    place(checkers, 2, 1, game::Color::Black);
    place(checkers, 2, 3, game::Color::Black);

    QVERIFY(checkers.hasLegalMoves(game::Color::White));
}

void LegalMoves::king() {
    auto checkers = cleanBoard();
    place(checkers, 4, 3, game::Color::White, true);

    QVERIFY(checkers.hasLegalMoves(game::Color::White));
}

void LegalMoves::kingBlocked() {
    auto checkers = cleanBoard();
    place(checkers, 4, 3, game::Color::White, true);

    place(checkers, 3, 2, game::Color::Black);
    place(checkers, 2, 1, game::Color::Black);
    place(checkers, 5, 4, game::Color::Black);
    place(checkers, 6, 5, game::Color::Black);
    place(checkers, 3, 4, game::Color::Black);
    place(checkers, 2, 5, game::Color::Black);
    place(checkers, 5, 2, game::Color::Black);
    place(checkers, 6, 1, game::Color::Black);

    QVERIFY(!checkers.hasLegalMoves(game::Color::White));
}

void LegalMoves::atEdge() {
    auto checkers = cleanBoard();

    place(checkers, 0, 1, game::Color::White);
    QVERIFY(!checkers.hasLegalMoves(game::Color::White));

    place(checkers, 7, 0, game::Color::Black);
    QVERIFY(!checkers.hasLegalMoves(game::Color::Black));
}

class GameState : public QObject {
    Q_OBJECT

private slots:
    void ongoingWithBothSidesMovable();
    void ongoingWhenOnlyOpponentIsStuck();
    void ongoingMidChain();

    void blackWinsWhenWhiteStuck();
    void whiteWinsWhenBlackStuck();

    void drawWhenQuietMovesReachPatience();
    void captureResetsQuietMoves();
    void promotionResetsQuietMoves();
    void drawTakesPriorityOverWin();
};

void GameState::ongoingWithBothSidesMovable() {
    auto checkers = cleanBoard();
    place(checkers, 3, 2, game::Color::White);
    place(checkers, 4, 3, game::Color::Black);

    QCOMPARE(checkers.state(), game::GameState::Ongoing);
}

void GameState::ongoingWhenOnlyOpponentIsStuck() {
    auto checkers = cleanBoard();
    place(checkers, 5, 2, game::Color::White);
    place(checkers, 7, 0, game::Color::Black);

    QVERIFY(!checkers.hasLegalMoves(game::Color::Black));
    QCOMPARE(checkers.state(), game::GameState::Ongoing);
}

void GameState::ongoingMidChain() {
    auto checkers = cleanBoard();
    place(checkers, 5, 2, game::Color::White);
    place(checkers, 4, 1, game::Color::Black);
    place(checkers, 2, 1, game::Color::Black);

    const auto state = checkers.perform({5, 2}, {{3, 0}, game::Position{4, 1}});

    QVERIFY(checkers.activePiece().has_value());
    QCOMPARE(state, game::GameState::Ongoing);
}

void GameState::blackWinsWhenWhiteStuck() {
    auto checkers = cleanBoard();
    place(checkers, 3, 2, game::Color::Black);

    QCOMPARE(checkers.state(), game::GameState::BlackWins);
}

void GameState::whiteWinsWhenBlackStuck() {
    auto checkers = cleanBoard();
    place(checkers, 5, 2, game::Color::White);
    place(checkers, 7, 0, game::Color::Black);

    const auto state = checkers.perform({5, 2}, {{4, 1}, std::nullopt});
    QCOMPARE(checkers.player(), game::Color::Black);
    QCOMPARE(state, game::GameState::WhiteWins);
}

void GameState::drawWhenQuietMovesReachPatience() {
    game::Checkers checkers(8, 0, 3);
    place(checkers, 5, 2, game::Color::White);
    place(checkers, 4, 5, game::Color::Black);

    checkers.perform({5, 2}, {{4, 1}, std::nullopt});
    checkers.perform({4, 5}, {{5, 4}, std::nullopt});
    const auto state = checkers.perform({4, 1}, {{3, 0}, std::nullopt});

    QVERIFY(checkers.hasLegalMoves(game::Color::White));
    QVERIFY(checkers.hasLegalMoves(game::Color::Black));
    QCOMPARE(state, game::GameState::Draw);
}

void GameState::captureResetsQuietMoves() {
    game::Checkers checkers(8, 0, 1);
    place(checkers, 5, 2, game::Color::White);
    place(checkers, 4, 1, game::Color::Black);
    place(checkers, 3, 6, game::Color::Black);

    const auto state = checkers.perform({5, 2}, {{3, 0}, game::Position{4, 1}});

    QCOMPARE(state, game::GameState::Ongoing);
}

void GameState::promotionResetsQuietMoves() {
    game::Checkers checkers(8, 0, 1);
    place(checkers, 1, 0, game::Color::White);
    place(checkers, 5, 4, game::Color::Black);

    const auto state = checkers.perform({1, 0}, {{0, 1}, std::nullopt});

    QCOMPARE(state, game::GameState::Ongoing);
}

void GameState::drawTakesPriorityOverWin() {
    game::Checkers checkers(8, 0, 1);
    place(checkers, 5, 2, game::Color::White);

    const auto state = checkers.perform({5, 2}, {{4, 1}, std::nullopt});

    QVERIFY(!checkers.hasLegalMoves(game::Color::Black));
    QCOMPARE(checkers.player(), game::Color::Black);
    QCOMPARE(state, game::GameState::Draw);
}

std::int32_t main(std::int32_t argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::int32_t status = 0;
    auto run = [&](QObject* test) { status |= QTest::qExec(test, argc, argv); };

    Board board; run(&board);
    SimpleMoves simple; run(&simple);
    CaptureMoves capture; run(&capture);
    ValidMoves valid; run(&valid);
    Perform perform; run(&perform);
    LegalMoves legal; run(&legal);
    GameState gameState; run(&gameState);

    return status;
}

#include "GameLogic.moc"