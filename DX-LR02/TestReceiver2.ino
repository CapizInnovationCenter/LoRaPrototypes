//using DX-LR02 and Esp32-C3 Super Mini, preparatation for protoboard

#include <HardwareSerial.h>

HardwareSerial LoRaSerial(1);

void setup() {
  Serial.begin(115200);

  // ESP32-C3 default UART pins: RX=20, TX=21
  LoRaSerial.begin(9600, SERIAL_8N1, 20, 21);

  Serial.println("Receiver Init...");

  // Configure LoRa module
  LoRaSerial.println("AT+FREQ=915000000");
  delay(300);
  LoRaSerial.println("AT+SF=7");
  delay(300);
  LoRaSerial.println("ATE0");
  delay(300);
  LoRaSerial.println("AT+RECV");
}

void loop() {
  if (LoRaSerial.available()) {
    String response = LoRaSerial.readStringUntil('\n');
    response.trim(); // remove whitespace/newlines

    if (response.startsWith("AT+SEND")) {
      Serial.println("Raw: " + response);

      // Split by commas
      int cntIndex = response.indexOf("CNT=");
      int tIndex   = response.indexOf("T=");
      int hIndex   = response.indexOf("H=");

      if (cntIndex > 0 && tIndex > 0 && hIndex > 0) {
        int cnt = response.substring(cntIndex + 4, response.indexOf(',', cntIndex)).toInt();
        int t   = response.substring(tIndex + 2, response.indexOf(',', tIndex)).toInt();
        int h   = response.substring(hIndex + 2).toInt();

        Serial.print("Counter: ");
        Serial.println(cnt);
        Serial.print("Temperature: ");
        Serial.println(t);
        Serial.print("Humidity: ");
        Serial.println(h);
        Serial.println("-------------------");
      }
    }
  }
}
