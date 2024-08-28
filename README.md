# QKlipper
Qt based Klipper/Moonraker library


## Usage
- Add QKlipper files to your Qt project
- Add Qt dependencies
```
QT += core network websockets
```
- Include QKlipperInstance
```
#include "QKlipper/QKlipperInstance/qklipperinstance.h"
```

### Local connections
```
    //Creating a local instance
    QKlipperInstance *instance = new QKlipperInstance();
    instance->setName("QKlipper Test");

    //address and port for non-rpc calls
    instance->setPort(7125);
    instance->setAddress("localhost"); //do not include http

    //instance location is required for local connections
    instance->setInstanceLocation("/home/parametheus/printer_data");
    instance->setAutoConnect(true);

    instance->init();
```
Local connections require `setInstanceLoction` to be called with the fully qualified path of the klipper installation.

### Remote connections
```
    //Creating a local instance
    QKlipperInstance *instance = new QKlipperInstance();
    instance->setName("QKlipper Test");

    //address and port for klipper instance
    instance->setPort(7125);
    instance->setAddress("artifex.local"); //do not include http
    instance->setAutoConnect(true);

    instance->init();
```
