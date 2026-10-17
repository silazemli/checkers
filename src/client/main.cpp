#include <QCoreApplication>

#include "networking/Utilities.hpp"
#include "client/Client.hpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    if (argc != 3) {
        qCritical("Usage: checkers_client <host> <port>");
        return EXIT_FAILURE;
    }

    try {
        QString host = QString::fromLocal8Bit(argv[1]);
        quint16 port = validatePort(QString::fromLocal8Bit(argv[2]));

        Client client(host, port);
        
        return app.exec();
    } catch (const std::exception& exception) {
        qCritical() << exception.what();
        return EXIT_FAILURE;
    }
}