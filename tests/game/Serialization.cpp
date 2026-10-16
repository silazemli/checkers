#include <array>
#include <optional>
#include <utility>
#include <vector>
#include <cstdint>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QtTest>

#include "game/Board.hpp"
#include "game/GameState.hpp"
#include "game/Move.hpp"
#include "game/Piece.hpp"
#include "game/Position.hpp"

namespace {

template <typename typeName>
typeName roundTrip(const typeName& value) {
    QByteArray buffer;

    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << value;

    QDataStream in(&buffer, QIODevice::ReadOnly);

    typeName result{};
    in >> result;

    if (in.status() != QDataStream::Ok)
        qWarning("round-trip read failed with QDataStream status %d", int(in.status()));

    return result;
}

} // namespace

class Position : public QObject {
    Q_OBJECT

private slots:
    void origin();
    void positive();
    void negative();
};

void Position::origin() {
    const game::Position value{0, 0};
    QCOMPARE(roundTrip(value), value);
}

void Position::positive() {
    const game::Position value{3, 5};
    QCOMPARE(roundTrip(value), value);
}

void Position::negative() {
    const game::Position value{-1, -2};
    QCOMPARE(roundTrip(value), value);
}

class Enum : public QObject {
    Q_OBJECT

private slots:
    void colors();
    void gameStates();
};

void Enum::colors() {
    QCOMPARE(roundTrip(game::Color::BLACK), game::Color::BLACK);
    QCOMPARE(roundTrip(game::Color::WHITE), game::Color::WHITE);
}

void Enum::gameStates() {
    QCOMPARE(roundTrip(game::GameState::ONGOING), game::GameState::ONGOING);
    QCOMPARE(roundTrip(game::GameState::DRAW), game::GameState::DRAW);
    QCOMPARE(roundTrip(game::GameState::WHITE_WINS), game::GameState::WHITE_WINS);
    QCOMPARE(roundTrip(game::GameState::BLACK_WINS), game::GameState::BLACK_WINS);
}

class Piece : public QObject {
    Q_OBJECT

private slots:
    void blackMan();
    void blackKing();
    void whiteMan();
    void whiteKing();
};

void Piece::blackMan() {
    const game::Piece value{game::Color::BLACK, false};
    QCOMPARE(roundTrip(value), value);
}

void Piece::blackKing() {
    const game::Piece value{game::Color::BLACK, true};
    QCOMPARE(roundTrip(value), value);
}

void Piece::whiteMan() {
    const game::Piece value{game::Color::WHITE, false};
    QCOMPARE(roundTrip(value), value);
}

void Piece::whiteKing() {
    const game::Piece value{game::Color::WHITE, true};
    QCOMPARE(roundTrip(value), value);
}

class Move : public QObject {
    Q_OBJECT

private slots:
    void withoutCapture();
    void withCapture();
};

void Move::withoutCapture() {
    const game::Move value{game::Position{5, 2}, std::nullopt};
    const game::Move result = roundTrip(value);

    QCOMPARE(result.to, value.to);
    QVERIFY(!result.captured.has_value());
}

void Move::withCapture() {
    const game::Move value{game::Position{3, 0}, game::Position{4, 1}};
    const game::Move result = roundTrip(value);

    QCOMPARE(result.to, value.to);
    QVERIFY(result.captured.has_value());
    QCOMPARE(*result.captured, *value.captured);
}

class Optional : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void populated();
};

void Optional::empty() {
    const std::optional<game::Position> value = std::nullopt;
    const std::optional<game::Position> result = roundTrip(value);

    QVERIFY(!result.has_value());
}

void Optional::populated() {
    const std::optional<game::Position> value = game::Position{7, -3};
    const std::optional<game::Position> result = roundTrip(value);

    QVERIFY(result.has_value());
    QCOMPARE(*result, *value);
}

class Vector : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void positions();
};

void Vector::empty() {
    const std::vector<game::Position> value;
    const std::vector<game::Position> result = roundTrip(value);

    QVERIFY(result.empty());
}

void Vector::positions() {
    const std::vector<game::Position> value = {
        {0, 0}, {1, 2}, {-3, -4}, {99, 100}
    };

    QCOMPARE(roundTrip(value), value);
}

class Board : public QObject {
    Q_OBJECT

private slots:
    void empty();
    void populated();
    void ragged();
};

void Board::empty() {
    const game::Board value;
    QCOMPARE(roundTrip(value), value);
}

void Board::populated() {
    const game::Board value = {
        { std::nullopt,
          game::Piece{game::Color::BLACK, false},
          std::nullopt,
          game::Piece{game::Color::BLACK, true} },
        { game::Piece{game::Color::WHITE, true},
          std::nullopt,
          std::nullopt,
          game::Piece{game::Color::WHITE, false} },
    };

    QCOMPARE(roundTrip(value), value);
}

void Board::ragged() {
    const game::Board value = {
        { std::nullopt },
        { game::Piece{game::Color::WHITE, false}, std::nullopt, std::nullopt },
        {},
    };

    QCOMPARE(roundTrip(value), value);
}

class Stream : public QObject {
    Q_OBJECT

private slots:
    void readPastEnd();
};

void Stream::readPastEnd() {
    QByteArray buffer;

    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << game::Position{1, 2};

    QDataStream in(&buffer, QIODevice::ReadOnly);

    game::Position first{};
    game::Position second{};
    in >> first;
    in >> second;

    QCOMPARE(in.status(), QDataStream::ReadPastEnd);
}

std::int32_t main(std::int32_t argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::int32_t status = 0;
    auto run = [&](QObject* test) { status |= QTest::qExec(test, argc, argv); };

    Position position; run(&position);
    Enum enums; run(&enums);
    Piece pieces; run(&pieces);
    Move moves; run(&moves);
    Optional optionals; run(&optionals);
    Vector vectors; run(&vectors);
    Board boards; run(&boards);
    Stream streams; run(&streams);

    return status;
}

#include "Serialization.moc"