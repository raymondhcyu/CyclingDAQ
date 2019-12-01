#include "BluetoothSerial.h"
#include "WiFi.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

String inMessage = "";

#define RXD2 16
#define TXD2 17

int i = 0;
byte buff[] = {0, 0, 0, 0, 0, 0, 0, 0}; // 8 byte message format
  
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  SerialBT.begin("The_Best_ESP32"); //Bluetooth device name
  WiFi.mode(WIFI_MODE_STA);

}

void loop() {
//  if (SerialBT.available()) {
//    char incomingChar = SerialBT.read();
//
//    if (incomingChar != '\n') {
//      inMessage += String(incomingChar);
//    }
//    else
//      inMessage = "";
//  }
//
//  if (inMessage == "Hello") {
//    SerialBT.println("Bye!");
//  }
//
//  int j = 0; // counter to change buffer data for testing
//  if ((inMessage == "Send me data") && (Serial1.available())) {
////    while (j < 50) {
//      // SerialBT.write() sends bytes
//      while (1)
//        SerialBT.write(Serial1.read());
//  
////      j++;
////      delay(100);
////    }
//  }

  if (Serial1.available() > 7) {
    byte startByte = Serial1.read();

    if (startByte == 255) {      
      SerialBT.write(startByte);
      Serial.println(startByte);
      
      for (i = 1; i < 8; i ++) {
        buff[i] = Serial1.read();
        SerialBT.write(buff[i]);
        Serial.println(buff[i]);
      }
    }
  }
  
//  while (Serial1.available()) {
//    Serial.println(Serial1.read());
//  }

}
