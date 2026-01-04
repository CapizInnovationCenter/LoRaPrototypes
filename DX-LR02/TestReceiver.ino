// Using M5 Atom and DX-LR02 915

#include <Adafruit_NeoPixel.h>
#include <HardwareSerial.h>

HardwareSerial LoRaSerial(1);

#define NEOPIXEL_PIN 27
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

int rxCounter = 0;
int lastSeq = 0;
unsigned long lastPacketTime = 0;

// Timeout control
const unsigned long expectedIntervalMs = 5000;  // transmitter sends every 5s
const unsigned long timeoutMarginMs   = 1000;   // grace margin
const unsigned long timeoutWindowMs   = expectedIntervalMs + timeoutMarginMs;
bool timeoutShown = false;
unsigned long lastTimeoutLog = 0;
const unsigned long timeoutLogThrottleMs = 2000;

// Extract payload after the second comma
String extractPayload(const String& msg) {
  if (msg.startsWith("AT+SEND=") || msg.startsWith("+RECV:")) {
    int firstComma = msg.indexOf(',');
    int secondComma = msg.indexOf(',', firstComma + 1);
    if (secondComma >= 0) {
      String p = msg.substring(secondComma + 1);
      p.trim();
      return p;
    }
  }
  return msg;
}

// Split payload into CNT/T/H tokens
void parsePayload(String payload, int &cntVal, float &tempVal, float &humVal) {
  cntVal = 0; tempVal = 0; humVal = 0;
  int start = 0;
  while (start < payload.length()) {
    int end = payload.indexOf(',', start);
    if (end < 0) end = payload.length();
    String token = payload.substring(start, end);
    token.trim();

    if (token.startsWith("CNT=")) {
      cntVal = token.substring(4).toInt();
    } else if (token.startsWith("T=")) {
      tempVal = token.substring(2).toFloat();
    } else if (token.startsWith("H=")) {
      humVal = token.substring(2).toFloat();
    }

    start = end + 1;
  }
}

void setup() {
  Serial.begin(115200);
  LoRaSerial.begin(9600, SERIAL_8N1, 22, 19); // RX=22, TX=19

  pixels.begin();
  pixels.clear();
  pixels.show();

  Serial.println("Receiver Init...");
  LoRaSerial.println("AT+FREQ=915000000");
  delay(300);
  LoRaSerial.println("AT+SF=7");
  delay(300);
  LoRaSerial.println("ATE0");
  delay(300);
  LoRaSerial.println("AT+RECV");

  lastPacketTime = millis();
}

void loop() {
  if (LoRaSerial.available()) {
    String msg = LoRaSerial.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      timeoutShown = false;
      pixels.clear();
      pixels.show();

      if (msg.startsWith("+RECV:") || msg.startsWith("AT+SEND=")) {
        rxCounter++;

        String payload = extractPayload(msg);
        payload.trim();

        Serial.print("Raw payload: ");
        Serial.println(payload);

        int cntVal; float tempVal; float humVal;
        parsePayload(payload, cntVal, tempVal, humVal);

        // Detect skipped sequence
        if (lastSeq > 0 && cntVal > 0 && cntVal != lastSeq + 1) {
          Serial.print("⚠️ Packet loss detected! Expected ");
          Serial.print(lastSeq + 1);
          Serial.print(" but got ");
          Serial.println(cntVal);

          pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // red blink
          pixels.show();
          delay(250);
          pixels.clear();
          pixels.show();
        }

        if (cntVal > 0) lastSeq = cntVal;
        lastPacketTime = millis();

        // Pretty print
        Serial.print("RX #");
        Serial.print(rxCounter);
        Serial.print(" → CNT=");
        Serial.print(cntVal);
        Serial.print(" | Temp=");
        Serial.print(tempVal, 2);
        Serial.print("°C | Hum=");
        Serial.print(humVal, 2);
        Serial.println("%");

        // Blue blink on receive
        pixels.setPixelColor(0, pixels.Color(0, 0, 255));
        pixels.show();
        delay(150);
        pixels.clear();
        pixels.show();
      }
    }
  }

  // Timeout watchdog
  unsigned long now = millis();
  if (now - lastPacketTime > timeoutWindowMs) {
    if (!timeoutShown) {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // solid red
      pixels.show();
      timeoutShown = true;
      lastTimeoutLog = 0;
    }
    if (now - lastTimeoutLog > timeoutLogThrottleMs) {
      Serial.println("No packet received in expected window!");
      lastTimeoutLog = now;
    }
  }

  delay(5);
}

