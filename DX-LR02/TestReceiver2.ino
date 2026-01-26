//using DX-LR02 and Esp32-C3 Super Mini, preparatation for protoboard - USE 3.3v
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
    response.trim();

    if (response.startsWith("AT+SEND")) {
      Serial.println("Raw: " + response);

      // Split into comma-separated tokens
      int lastComma = response.indexOf(',');
      String payload = response.substring(lastComma + 1); // after AT+SEND=255,xx,

      int counter = -1, temp = -1, hum = -1;

      // Break payload into fields
      int start = 0;
      while (start < payload.length()) {
        int comma = payload.indexOf(',', start);
        if (comma == -1) comma = payload.length();
        String field = payload.substring(start, comma);
        field.trim();

        if (field.startsWith("CNT=")) {
          counter = field.substring(4).toInt();
        } else if (field.startsWith("T=")) {
          temp = field.substring(2).toInt();
        } else if (field.startsWith("H=")) {
          hum = field.substring(2).toInt();
        }

        start = comma + 1;
      }

      Serial.print("Counter: ");
      Serial.println(counter);
      Serial.print("Temperature: ");
      Serial.println(temp);
      Serial.print("Humidity: ");
      Serial.println(hum);
      Serial.println("-------------------");
    }
  }
}
