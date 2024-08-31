#include <QCoreApplication>

#include "QKlipperInstance/qklipperinstance.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //Creating a local instance
    QKlipperInstance *instance = new QKlipperInstance();
    instance->setName("QKlipper Test");

    //address and port for non-rpc calls
    instance->setPort(7125);
    instance->setAddress("localhost");

    //instance location is required for local connections
    instance->setInstanceLocation("/home/parametheus/printer_data");

    instance->connect();

    return a.exec();
}
