
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;
int flexRes = 34;
int flexReading;
int strainRes = 32;
int strainReading;
int flexBase = 0;
int strainBase = 0;
int approval = 0;

#define bleServerName "IMU-Data"
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

void setup() {
        Serial.begin(115200);
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
                      pCharacteristic->addDescriptor(new BLE2902());
                    
                      // Start the service
                      pService->start();
                    
                      // Start advertising
                      pServer->getAdvertising()->start();
                      Serial.println("Waiting a client connection to notify...");
}
void loop(){
  flexReading = analogRead(flexRes);
  strainReading = analogRead(strainRes);
  Serial.print("flex Reading is: ");
  Serial.println(flexReading);
  Serial.print("strain Reading is: ");
  Serial.println(strainReading);
  if(deviceConnected){
      flexReading = analogRead(flexRes);
      strainReading = analogRead(strainRes);
      Serial.print("Reading is: ");
      Serial.println(flexReading);
      if(strainReading > 1650){
        static char Reading[7];
        approval = 100;
        dtostrf(approval,6,2,Reading);
        Serial.print("APPROVAL IS:");
        Serial.println(approval);
        pCharacteristic->setValue(Reading);
        pCharacteristic->notify();
        approval = 0;
      }
  }
  delay(250);
}
