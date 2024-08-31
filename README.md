![alt text](https://github.com/DigitalArtifex/QKlipper/blob/main/qklipper_small.jpg)
# QKlipper
Qt based Klipper/Moonraker library (ALPHA - convenience methods still being ported over the next couple of days as well as QML compatability) 


## Setup
- Add QKlipper files to your Qt project
- Add Qt dependencies
```
    QT += core network websockets
```
- Include QKlipperInstance
```
    #include "QKlipper/QKlipperInstance/qklipperinstance.h"
```
##
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
    instance->setAddress("artifex.local"); //do not include http
    
    instance->connect();
```


Once `init` has been called, the QKlipperInstance will create the **QKlipperServer**, **QKlipperPrinter**, **QKlipperConsole** and **QKlipperSystem** and if auto connect is set to true, the QKlipperConole will begin the startup sequence.

##

## Usage
In order to interface with the Klipper instance we need to make sure the console has connected and syncronized. The easiest way to do this is to connect to
the **QKlipperConsole::connectionStateChanged()** signal and check for the *Syncronized* state and that it's not still connecting.
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
            QObject::connect(instance->console(), 
                            SIGNAL(connectionStateChanged()), 
                            this, 
                            SLOT(onConsoleConnectionStateChanged())
                            );
            
            //set the printer object
            printer = instance->printer();
            
            //connect to Klipper
            instance->connect();
        }
    
    protected slots:
        void onConsoleConnectionStateChanged()
        {
            if(console->hasConnectionState(QKlipperConsole::Syncronized) && !console->isConnecting())
            {
                //console is ready
                printer->toolhead()->home(); //homes the toolhead
                printer->toolhead()->setPosition(100, 50, 50); //sets absolute position
                printer->bed()->setTargetTemperature(60); //sets the target temp (C)
            }
        }
};
```
