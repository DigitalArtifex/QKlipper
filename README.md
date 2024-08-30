![alt text](https://github.com/DigitalArtifex/QKlipper/blob/main/qklipper_small.jpg)
# QKlipper
Qt based Klipper/Moonraker library (ALPHA - convenience methods still being ported over the next couple of days as well as QML compatability) 


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
    //Creating a remote instance
    QKlipperInstance *instance = new QKlipperInstance();
    instance->setName("QKlipper Test");

    //address and port for klipper instance
    instance->setPort(7125);
    instance->setAddress("artifex.local"); //do not include http
    instance->setAutoConnect(true);

    instance->init();
```


Once `init` has been called, it is then possible to connect to the various signals provided by QKlipperServer, QKlipperPrinter and QKlipperSystem. These classes also provide various convenience functions for controlling the different aspects of the Klipper instance. QKlipperConsole also provides methods for nearly the entire Moonraker API and supports sending custom messages (custom parser hooks coming soon).

```
    QKlipperPrinter *printer = instance->printer();
    printer->toolhead()->home();
```
