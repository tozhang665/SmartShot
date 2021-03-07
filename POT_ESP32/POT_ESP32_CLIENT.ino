/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include "BLEDevice.h"
#include <Wire.h>

//Default Temperature is in Celsius
//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "POTmeter"

static BLEUUID SERVICE_UUID("1a2306b1-ec0b-41d8-9970-01869646b99a");
static BLEUUID CHARACTERISTIC_UUID("216ea919-35a4-4037-876a-9132e5eadd4a");

//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning... Hopefully.
static BLEAddress *pServerAddress;
 
//Characteristic that we want to read and characteristic that we want to write.
static BLERemoteCharacteristic* potCharacteristic;

const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

char* potR;

boolean newPotR = false;


int myPot = 34;
int currentVal = 0;
int setupResArray[50];
int actualResArray[50];
int setupResAverage = 0;
int actualResAverage = 0;




//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
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
  potCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);

  if (potCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristic");
 
  //Assign callback functions for the Characteristics
  potCharacteristic->registerForNotify(potNotifyCallback);
  return true;
}

//Callback function that gets called, when another device's advertisement has been received
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

 
//When the BLE Server sends a new temperature reading with the notify property
static void potNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  potR = (char*)pData;
  newPotR = true;
}

//function that prints the latest sensor readings in the OLED display

void setup() {


  
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






  //Start serial communication
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  //Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
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
 



  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      potCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  //if new temperature readings are available, print in the OLED
  if (newPotR){
    newPotR = false;
    Serial.print("CLIENT RECEIVED THE FOLLOWING: ");
    Serial.println(potR);
  }

  if((int)potR == 0 && actualResAverage ==0){
    //NOT GOOD
  }else if((int)potR == 100 && actualResAverage == 0 || (int)potR == 0 && actualResAverage == 100){
    //ONLY 1 GOOD
  }else if((int)potR == 100 && actualResAverage == 100){
    //BOTH GOOD
  }

  
  delay(1000); // Delay a second between loops.
}
