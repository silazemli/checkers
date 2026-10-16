cmake_minimum_required(VERSION 3.20)

project(Checkers LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(
    Qt6 REQUIRED COMPONENTS
        Core
        Network
        Test )

add_library(
    checkers
        src/game/Board.cpp
        src/game/Checkers.cpp
        src/game/GameState.cpp
        src/game/Move.cpp
        src/game/Color.cpp
        src/game/Piece.cpp
        src/game/Position.cpp )
target_include_directories(
    checkers
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include )
target_link_libraries(
    checkers
    PUBLIC
        Qt6::Core )

add_library(
    networking
        src/networking/Actions.cpp
        src/networking/Message.cpp
        src/networking/Serialization.cpp
        src/networking/Utilities.cpp )
target_include_directories(
    networking
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include )
target_link_libraries(
    networking
    PUBLIC
        checkers
        Qt6::Core )

add_executable(
    checkers_server
        src/server/main.cpp
        src/server/Server.cpp )
target_sources(
    checkers_server
    PRIVATE
        include/server/Server.hpp )
target_link_libraries(
    checkers_server
    PRIVATE
        networking
        Qt6::Network )

add_executable(
    checkers_client
        src/client/main.cpp
        src/client/Client.cpp )
target_sources(
    checkers_client
    PRIVATE
        include/client/Client.hpp )
target_link_libraries(
    checkers_client
    PRIVATE
        networking
        Qt6::Network )

function(checkers_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

checkers_add_test(game_logic_tests tests/game/GameLogic.cpp)
target_link_libraries(game_logic_tests PRIVATE checkers)

checkers_add_test(serialization_tests tests/game/Serialization.cpp)
target_link_libraries(serialization_tests PRIVATE checkers)

checkers_add_test(framing_tests tests/networking/Framing.cpp)
target_link_libraries(framing_tests PRIVATE networking)

checkers_add_test(client_server_tests
    tests/networking/ClientServer.cpp
    src/server/Server.cpp
    src/client/Client.cpp )
target_sources(
    client_server_tests
    PRIVATE
        include/server/Server.hpp
        include/client/Client.hpp )
target_link_libraries(
    client_server_tests
    PRIVATE
        networking
        Qt6::Network )