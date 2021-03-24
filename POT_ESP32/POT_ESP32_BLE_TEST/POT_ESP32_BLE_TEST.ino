
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "HX711.h"

#include<Wire.h>

//HX711 DATA
HX711 scale;

int dataPin = 21;
int clockPin = 22;
float reading = 0;
int base = 0;


//GY DATA
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;


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
int setupStrainArray[50];
int actualStrainArray[50];
int setupStrainAverage = 0;
int actualStrainAverage = 0;


void setup() {
  Serial.begin(115200);

  //GYRO START
  
  //Wire.begin();
  //Wire.beginTransmission(MPU_addr);
  //Wire.write(0x6B);  // PWR_MGMT_1 register
  //Wire.write(0);     // set to zero (wakes up the MPU-6050)
  //Wire.endTransmission(true);
  
  //STRAIN GAUGE START
  scale.begin(dataPin, clockPin);


   for(int i = 0; i < sizeof(setupStrainArray); i++){
    setupStrainAverage = scale.read() / 100000;
    delay(20);
  }
  setupStrainAverage = setupStrainAverage / 50;
  









  

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



  for(int i = 0; i < sizeof(actualStrainArray); i++){
    actualStrainAverage = scale.read() / 100000;
    delay(20);
  }
  actualStrainAverage = actualStrainAverage / 50;
  



  //TAKE THE VALUES FROM THE GYRO

  //Wire.beginTransmission(MPU_addr);
  //Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  //Wire.endTransmission(false);
  //Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  //AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  //AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  //AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  //Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  //GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  //GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  //GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  //Serial.println("AcX = "); Serial.print(AcX);
  //Serial.println("AcY = "); Serial.print(AcY);
  //Serial.println("AcZ = "); Serial.print(AcZ);








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
