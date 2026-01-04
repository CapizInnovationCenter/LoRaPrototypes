// Using M5 Atom and DX-LR02 915

#include <Adafruit_NeoPixel.h>
#include <HardwareSerial.h>
#include <DHT.h>

HardwareSerial LoRaSerial(1);
int txCounter = 0;

#define NEOPIXEL_PIN 27
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// DHT11 setup
#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// DHT11 timing
const unsigned long dhtMinIntervalMs = 2000; // DHT11 needs ~2s
unsigned long lastDhtRead = 0;

bool readDHT(float &t, float &h) {
  // Enforce min interval
  unsigned long now = millis();
  if (now - lastDhtRead < dhtMinIntervalMs) {
    delay(dhtMinIntervalMs - (now - lastDhtRead));
  }

  // Try up to 3 times
  for (int i = 0; i < 3; i++) {
    h = dht.readHumidity();
    t = dht.readTemperature(); // Celsius
    if (!isnan(t) && !isnan(h)) {
      lastDhtRead = millis();
      return true;
    }
    delay(150);
  }
  lastDhtRead = millis();
  return false;
}

void setup() {
  Serial.begin(115200);
  LoRaSerial.begin(9600, SERIAL_8N1, 22, 19); // RX=22, TX=19

  pixels.begin();
  pixels.clear();
  pixels.show();

  dht.begin();

  Serial.println("Transmitter Init...");
  LoRaSerial.println("AT+FREQ=915000000");
  delay(400);
  LoRaSerial.println("AT+SF=7");
  delay(400);

  // Warm-up: discard first read
  float t, h;
  readDHT(t, h);
}

void loop() {
  txCounter++;

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  // Fallbacks if NaN
  if (isnan(temp)) temp = 0;
  if (isnan(hum))  hum  = 0;

  // Build payload with ALL fields
  String payload = "CNT=" + String(txCounter) +
                   ",T=" + String((int)temp) +
                   ",H=" + String((int)hum);

  String cmd = "AT+SEND=255," + String(payload.length()) + "," + payload;

  LoRaSerial.println(cmd);
  Serial.print("TX #");
  Serial.print(txCounter);
  Serial.print(" → ");
  Serial.println(payload);

  // Blink NeoPixel green on send
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  delay(150);
  pixels.clear();
  pixels.show();

  delay(5000); // send every 5 seconds
}


