
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;


#define bleServerName "POTmeter"
#define SERVICE_UUID "1a2306b1-ec0b-41d8-9970-01869646b99a"
#define CHARACTERISTIC_UUID "216ea919-35a4-4037-876a-9132e5eadd4a"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


int myPot = 34;
int currentVal = 0;
int setupResArray[50];
int actualResArray[50];
int setupResAverage = 0;
int actualResAverage = 0;


void setup() {
  Serial.begin(115200);



  
  // put your setup code here, to run once:
  //grab 50 values that are taken over 5 seconds 
  for(int i = 0; i < sizeof(setupResArray); i++){
    setupResArray[i] = analogRead(myPot);
    delay(100);
  }
  // find the average of the previous values;
  for(int j : setupResArray){
    setupResAverage = setupResAverage + j;
  }
  setupResAverage = setupResAverage/50;






  

  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  
}

void loop() {







  
  // take 50 readings over 1 second 
  for(int i = 0; i < sizeof(actualResArray); i++){
    actualResArray[i] = analogRead(myPot);
    delay(20);
  }
  for(int j : actualResArray){
    actualResAverage = actualResAverage + j;
  }
  actualResAverage = actualResAverage / 50;

  // send 100 when reading is good, send 0 if not good. 






  
  if (deviceConnected) {




    
  if(actualResAverage > (.9 * setupResAverage) && actualResAverage < (1.1 * setupResAverage)){
    value = 100;
  }else{
    value = 0;
  }




  
    static char potReading[7];
    dtostrf(value,6,2,potReading);
    Serial.printf("*** NOTIFY: %d ***\n", value);
    pCharacteristic->setValue(potReading);
    pCharacteristic->notify();
    //pCharacteristic->indicate();
  }
  delay(2000);

  
}
