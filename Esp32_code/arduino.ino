#include <FirebaseESP32.h>
#include <BLEDevice.h> //Header file for BLE 
#include <WiFi.h>
#include <ESPmDNS.h>
#define mem 20
static BLEUUID serviceUUID("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"); 
static BLEUUID    charUUID("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxx"); 
String My_BLE_Address = "xx:xx:xx:xx:xx:xx"; 
static BLERemoteCharacteristic* pRemoteCharacteristic;
BLEScan* pBLEScan; 
BLEScanResults foundDevices;
static BLEAddress *Server_BLE_Address;
int ind=0;
String Scaned_BLE_Address[20];
int Scaned_BLE_Rssi[20];
static int led1=23;
static int led2=22;
boolean on1 = false; 
boolean on2 = false; 
boolean manual = false;
int countdown=5;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Scan Result: %s \n", advertisedDevice.toString().c_str());
      Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
      Scaned_BLE_Address[ind%mem] = Server_BLE_Address->toString().c_str();
      Scaned_BLE_Rssi[(ind++)%mem]=advertisedDevice.getRSSI();
    }

};
#define WIFI_SSID "xxxxxxxx" // your wifi SSID
#define WIFI_PASSWORD "xxxxxxxxxx" //your wifi PASSWORD

#define FIREBASE_HOST "xxxx.firebasedatabase.app" // change here
#define FIREBASE_AUTH "xxxxxxxxxxxxxxxxxxxxxxxxx"// change here
FirebaseData firebaseData;

void setup ()
{
    Serial.begin(115200); //Start serial monitor 
    Serial.println("ESP32 sensor device active");
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); 
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); 
    pBLEScan->setActiveScan(true);
    pinMode (led1,OUTPUT); 
    pinMode (led2,OUTPUT); 
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting to wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ") ;
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);     
}
void loop ()
{
  if(Firebase.getString(firebaseData, "/mcb"))
    {
      String manualStatus = firebaseData.stringData();
      if(manualStatus.toInt() == 1){
        Serial.println("manual mode on");
          manual=true;
      }
      else {
        Serial.println("manual off");
        manual=false;
      }
    }
  if(manual){
    if(Firebase.getString(firebaseData, "/Led1Status"))
    {
      String ledstatus1 = firebaseData.stringData();
      if(ledstatus1.toInt() == 1){
        Serial.println("on1");
          digitalWrite(led1,HIGH);
          on1=true;
      }
      else {
        Serial.println("off1");
        digitalWrite(led1,LOW);
        on1=false;
      }
    }
    else{
      Serial.print("Error in getInt, ");
      Serial.println(firebaseData.errorReason());
    } 
    if(Firebase.getString(firebaseData, "/PCStatus"))
    {
      String ledstatus2 = firebaseData.stringData();
      if(ledstatus2.toInt() == 1){
        Serial.println("ON2");
          digitalWrite(led2,HIGH);
          on2=true;
      }
      else {
        Serial.println("off2");
        digitalWrite(led2,LOW);
        on2=false;
      }
    }
    else{
      Serial.print("Error in getInt, ");
      Serial.println(firebaseData.errorReason());
    } 
  }
  if(!manual){
  boolean found=false;
  Serial.println(",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,");
  // Serial.println(Scaned_BLE_Address);
  Serial.println(",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,");

  foundDevices = pBLEScan->start(3); //Scan for 3 seconds to find the Fitness band 
  if (foundDevices.getCount() >= 1){
    Serial.println(foundDevices.getCount());
    // for(int i=ind-Serial.println(foundDevices.getCount())-2;i<ind;i++){
    for(int i=0;i<mem;i++){
      if(Scaned_BLE_Address[i]=="")continue;
      Serial.println("processing");
      Serial.println(Scaned_BLE_Address[i]);
      Serial.println(Scaned_BLE_Rssi[i]);
      if (Scaned_BLE_Address[i] == My_BLE_Address ){
        if(Scaned_BLE_Rssi[i]<-80){
          Serial.println("Signal too week!");
          break;
        }
        found=true;
        Serial.println("Found Device :-)...");
        break;
      }
    }
  } 
  // break;
  if(found && !on1){
    
    Serial.println("connecting to Server as client");
    Serial.println("********************LED turned ON************************");
    digitalWrite (led1,HIGH);
    countdown=5;
    on1=true;
  }
  else if(!found && on1 ){
    countdown--;
    Serial.print("bluetooth signal lost. countdown:");
    Serial.println(countdown);
    if(countdown<=0){
      Serial.println("********************LED turned OFF************************");
      digitalWrite (led1,LOW);
      on1=false;
    }
  }
  if(found && !on2){
    
    Serial.println("connecting to Server as client");
    Serial.println("********************LED turned ON************************");
    digitalWrite (led2,HIGH);
    countdown=5;
    on2=true;
  }
  else if(!found && on2 ){
    countdown--;
    Serial.print("bluetooth signal lost. countdown:");
    Serial.println(countdown);
    if(countdown<=0){
      Serial.println("********************LED turned OFF************************");
      digitalWrite (led2,LOW);
      on2=false;
    }
  }
    memset(Scaned_BLE_Address, 0, sizeof(Scaned_BLE_Address));
 }
}
