#include <QCoreApplication>

#include "networking/Utilities.hpp"
#include "server/Server.hpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    if (argc != 2) {
        qCritical("Usage: checkers_server <port>");
        return EXIT_FAILURE;
    }

    try {
        quint16 port = validatePort(QString::fromLocal8Bit(argv[1]));
        Server server(port);

        return app.exec();
    } catch (const std::exception& exception) {
        qCritical() << exception.what();
        return EXIT_FAILURE;
    }
}