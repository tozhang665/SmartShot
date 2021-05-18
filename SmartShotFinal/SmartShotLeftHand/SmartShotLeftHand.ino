
#include "BLEDevice.h"
#include <Wire.h>
#include "HX711.h"


//BLE server Name
#define bleServerName "IMU-Data"

int dataPin = 21;
int clockPin = 22;
HX711 scale;
int fsrReading;
int fsrBase = 0;
int scaleReading;
int scaleBase;
int respin = 27;
 

int LED1 = 32;
int LED2 = 33;
int LED3 = 25;


static BLEUUID SERVICE_UUID("1a2306b1-ec0b-41d8-9970-01869646b99a");
static BLEUUID CHARACTERISTIC_UUID("216ea919-35a4-4037-876a-9132e5eadd4a");

static boolean doConnect = false;
static boolean connected = false;

static BLEAddress *pServerAddress;
 
static BLERemoteCharacteristic* IMU_Characteristic;

char* imuReading;
boolean newImuR = false;

const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};


                          
                      bool connectToServer(BLEAddress pAddress) {
                        Serial.println("connection Call");
                             BLEClient* pClient = BLEDevice::createClient();
                           
                            // Connect to the remove BLE Server.
                            pClient->connect(pAddress);
                            Serial.println(" - Connected to server");
                           
                            // Obtain a reference to the service we are after in the remote BLE server.
                            BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
                            if (pRemoteService == nullptr) {
                              Serial.print("Failed to find our service UUID: ");
                              Serial.println(SERVICE_UUID.toString().c_str());
                              return (false);
                            }
                           
                            // Obtain a reference to the characteristics in the service of the remote BLE server.
                            IMU_Characteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
                          
                            if (IMU_Characteristic == nullptr) {
                              Serial.print("Failed to find our characteristic UUID");
                              return false;
                            }
                            Serial.println(" - Found our characteristic");
                           
                            //Assign callback functions for the Characteristics
                            IMU_Characteristic->registerForNotify(IMUNotifyCallback);
                            return true;
                          }
                          
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
  }
};                          
                    //When the BLE Server sends a new resistance reading with the notify property
                    static void IMUNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                                            uint8_t* pData, size_t length, bool isNotify) {
                      //store temperature value
                      imuReading = (char*)pData;
                      newImuR = true;
                    }

void setup() {

  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);

  scale.begin(dataPin, clockPin);
    
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");


  
                  //BLE SETUP PHASE
                  BLEDevice::init("");
                  Serial.println("started init");
                  // Retrieve a Scanner and set the callback we want to use to be informed when we
                  // have detected a new device.  Specify that we want active scanning and start the
                  // scan to run for 30 seconds.
                  BLEScan* pBLEScan = BLEDevice::getScan();
                  
                  Serial.println("started scan");
                  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
                  
                  Serial.println("started callback");
                  pBLEScan->setActiveScan(true);
                  
                  Serial.println("started active scan");

                  
        digitalWrite(LED1,HIGH);
        digitalWrite(LED2,HIGH);
        digitalWrite(LED3,HIGH);
                  pBLEScan->start(15);
        digitalWrite(LED2,LOW);
        digitalWrite(LED3,LOW);
        delay(2500);
        scaleBase = analogRead(respin);
        digitalWrite(LED2,HIGH);
        digitalWrite(LED3,HIGH);
        delay(2500);
        digitalWrite(LED2,LOW);
        digitalWrite(LED3,LOW);
}

void loop() {
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      IMU_Characteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }


  if (newImuR){
    newImuR = false; 
    Serial.print("CLIENT RECEIVED THE FOLLOWING: ");
    fsrReading = (int)imuReading;
    Serial.println(fsrReading);
  }
  scaleReading = analogRead(respin);
  Serial.print("Base is: ");
  Serial.println(scaleBase);
  Serial.print("Reading is: ");
  Serial.println(scaleReading);
  Serial.print("IMU READING: ");
  Serial.println(fsrReading);
  if(scaleReading < scaleBase + 50 && scaleReading > scaleBase - 50 && fsrReading > 100){
    digitalWrite(LED2,HIGH);
    digitalWrite(LED3,HIGH);
    }
  else if(scaleReading < scaleBase + 50 && scaleReading > scaleBase - 50 && fsrReading <= 100){
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,HIGH);
   }
   else{
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
   }
  delay(250);
}
