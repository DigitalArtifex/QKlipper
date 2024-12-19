![alt text](https://github.com/DigitalArtifex/QKlipper/blob/main/qklipper_small.jpg)
# QKlipper
Qt based Klipper/Moonraker library (ALPHA - convenience methods still being ported over the next couple of days as well as QML compatability) 


## Setup
- Add QKlipper files to your Qt project
- Add Qt dependencies
```
    QT += core network websockets
```
- Include QKlipper
```
    #include "QKlipper/qklipper.h"
```
##
### Local connections
```
    //Creating a local instance
    QKlipperInstance *instance = new QKlipperInstance();
    instance->setName("QKlipper Test");
    
    //address and port for non-rpc calls
    instance->setPort(7125);
    instance->setAddress("http://localhost");
    
    //instance location is required for local connections
    instance->setInstanceLocation("/home/parametheus/printer_data");
    
    instance->connect();
```
Local connections require `setInstanceLoction` to be called with the fully qualified path of the klipper installation.
##
### Remote connections
```
    //Creating a remote instance
    QKlipperInstance *instance = new QKlipperInstance();
    instance->setName("QKlipper Test");
    
    //address and port for klipper instance
    instance->setPort(7125);
    instance->setAddress("http://artifex"); //it is best to use the hostname, IP or full domain name. Avoid using .local
    
    instance->connect();
```


Once `QKlipperInstance->connect()` has been called the QKlipperConole will attempt to connect to the sockets and begin the startup sequence.

##

## Usage
In order to interface with the Klipper instance we need to make sure the console has connected and syncronized. The easiest way to do this is to connect to
the **QKlipperInstance::connected()** signal. If you want, you can instead connect to the **QKlipperConsole::connectionStateChanged()** to react to the individual
states of the connection process.
```
class MyClass : QObject
{
    Q_OBJECT
    QKlipperPrinter *printer = nullptr;
    QKlipperInstance *instance = nullptr;
    
    public:
        MyClass(QObject *parent = nullptr) : QObject{parent}
        {
            //Creating a local instance
            instance = new QKlipperInstance();
            instance->setName("QKlipper Test");
            
            //address and port for non-rpc calls
            instance->setPort(7125);
            instance->setAddress("localhost"); //do not include http
            
            //instance location is required for local connections
            instance->setInstanceLocation("/home/parametheus/printer_data");
            
            //connect to the signal
            connect(instance, SIGNAL(connected), this, SLOT(onInstanceConnected()));
            
            //set the printer object
            printer = instance->printer();
            
            //connect to Klipper
            instance->connect();
        }
    
    protected slots:
        void onInstanceConnected()
        {
            //console is ready
            printer->toolhead()->home(); //homes the toolhead
            printer->toolhead()->setPosition(100, 50, 50); //sets absolute position
            printer->bed()->setTargetTemperature(60); //sets the target temp (C)
        }
};
```
