#include <SakuraIO.h>
 
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 3, TXPin = 2;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;
 
// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

SakuraIO_I2C sakuraio;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ss.begin(GPSBaud);
 
  //pinMode(5, OUTPUT); //スリープモードピン 
  //digitalWrite(5, LOW); //スリープモード ON

// Check module
  uint16_t productId = sakuraio.getProductID();
  switch(productId){
    case 0x0001:
      Serial.println("SCM-LTE-Beta");
      break;
    case 0x0002:
      Serial.println("SCM-LTE-01");
      break;
    default:
      Serial.println("Please check connection");
      return 1;
  }
  
  // FirmwareCheck
  uint8_t ret;
  char version[33] = {0};
  Serial.println("Get current version");
  ret = sakuraio.getFirmwareVersion(version);
  Serial.print("version");
  Serial.print(version);
  Serial.print("ret");
  Serial.println(ret);

  // 通信状態のチェック
  for(;;){
    if( (sakuraio.getConnectionStatus() & 0x80) == 0x80 ) break;
    Serial.print(".");
    Serial.print("getConnectionStatus="); Serial.println(sakuraio.getConnectionStatus());
    delay(1000);
  }
  Serial.print("Set up Complete...");
}

void loop() {

  while (ss.available() > 0){
    char c = ss.read();
    //Serial.print(c);
    gps.encode(c);
    if (gps.location.isUpdated()){
      Serial.print("unixtime="); Serial.println(gps.time.value());
      Serial.print("LAT="); Serial.println(gps.location.lat(), 6);
      Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
      Serial.print("ALT="); Serial.println(gps.altitude.meters());
      Serial.print("satellites="); Serial.println(gps.satellites.value());
      Serial.print("hdop="); Serial.println(gps.hdop.value());
      Serial.print("speed="); Serial.println(gps.speed.value());
      Serial.println("==================================");

      sakuraio.enqueueTx(0, gps.time.value()); // ch3 unixtime
      sakuraio.enqueueTx(1, gps.location.lat());      //ch0 緯度
      sakuraio.enqueueTx(2, gps.location.lng());      //ch1 経度
      sakuraio.enqueueTx(3, gps.satellites.value());   //ch2 受信衛星数
      sakuraio.enqueueTx(4, gps.hdop.value());    //ch4 hdop
      sakuraio.send();
      Serial.println("send!");
      
    }
  }
}
