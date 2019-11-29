//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"
#include "WiFi.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

String inMessage = "";

byte buff[] = {255, 50, 100, 150, 200, 50, 100, 150}; // 8 byte message format

void setup() {
  Serial.begin(115200);
  SerialBT.begin("The_Best_ESP32"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
  WiFi.mode(WIFI_MODE_STA);
  Serial.print("MAC address is this - ");
  Serial.println(WiFi.macAddress());
}

void loop() {  
  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();

    if (incomingChar != '\n') {
      inMessage += String(incomingChar);
    }
    else
      inMessage = "";
  }

  if (inMessage == "Hello") {
    Serial.println("Received hello!");
    SerialBT.println("Bye!");
  }

  int j = 0; // counter to change buffer data for testing
  if (inMessage == "Send me data") {
    while (j < 50) {
      // SerialBT.write() sends bytes
      Serial.print("Ok sending back data! ");
      SerialBT.write(buff[0]);
      Serial.print(buff[0]);
      SerialBT.write(buff[1] + j);
      Serial.print(buff[1] + j);
      SerialBT.write(buff[2] + j);
      Serial.print(buff[2] + j);
      SerialBT.write(buff[3] + j);
      Serial.print(buff[3] + j);
      SerialBT.write(buff[4] + j);
      Serial.print(buff[4] + j);
      SerialBT.write(buff[5] + j);
      Serial.print(buff[5] + j);
      SerialBT.write(buff[6] + j);
      Serial.print(buff[6] + j);
      SerialBT.write(buff[7] + j);
      Serial.print(buff[7] + j);
  
      j++;
      delay(500);
    }
//      for (int i = 0; i < sizeof(buff); i++) {
//        if (buff[i] = 0) {
//          SerialBT.write(buff[i]);
//        }
//        else {
//          SerialBT.write(buff[i] + j);
//        }
//      }
  }
}
