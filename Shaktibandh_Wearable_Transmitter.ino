#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <time.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// SHAKTIBANDH WEARABLE - NodeMCU ESP8266
// Button D5->GND | WS2812 DIN D2 | vibration IN D7
// OLED SDA D6, SCL D1, address 0x3C
// GPS TX -> D3 (GPS RX unused). D3/GPIO0 must not be held LOW at boot.

#define SOS_BUTTON D5
#define LED_PIN D2
#define VIBRATION_PIN D7
#define GPS_RX D3
#define NUM_LEDS 64
#define WIFI_CHANNEL 1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial gpsSerial(GPS_RX, -1);
TinyGPSPlus gps;
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "RadheRadhe";
const char* password = "radhekrishna";
const char* timezone = "IST-5:30";
const time_t VALID_TIME = 1760000000;

uint8_t gatewayMAC[] = {0x48, 0x3F, 0xDA, 0x47, 0x2F, 0x4C};

double latitude = 0, longitude = 0;
bool gpsFix = false, timeIsValid = false;
bool emergencyLightActive = false, vibrationActive = false;
unsigned long lastGPSStatus = 0, lastClockUpdate = 0;
unsigned long vibrationStart = 0;
const unsigned long vibrationDuration = 3000;

bool lastButtonReading = HIGH, stableButtonState = HIGH;
unsigned long lastDebounceTime = 0, firstPressTime = 0;
bool doublePressHandled = false;
const unsigned long debounceTime = 50, doublePressWindow = 3000;

void showMessage(const char* a, const char* b) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 18); display.println(a);
  display.setCursor(0, 36); display.println(b);
  display.display();
}

void readGPS() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());
  gpsFix = gps.location.isValid() && gps.location.age() < 10000;
  if (gpsFix) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  }

  if (millis() - lastGPSStatus >= 2000) {
    lastGPSStatus = millis();
    if (gpsFix) {
      Serial.println("GPS FIX ACQUIRED");
      Serial.print("Latitude: "); Serial.println(latitude, 6);
      Serial.print("Longitude: "); Serial.println(longitude, 6);
      Serial.print("Fix age ms: "); Serial.println(gps.location.age());
    } else {
      Serial.println("GPS: NO RECENT FIX");
      Serial.print("GPS characters: "); Serial.println(gps.charsProcessed());
    }
  }
}

bool getFreshGPSFix() {
  readGPS();
  gpsFix = gps.location.isValid() && gps.location.age() < 10000;
  if (!gpsFix) return false;
  latitude = gps.location.lat();
  longitude = gps.location.lng();
  return latitude >= -90 && latitude <= 90 &&
         longitude >= -180 && longitude <= 180;
}

void sendESPNowAlert() {
  char packet[80];
  if (getFreshGPSFix())
    snprintf(packet, sizeof(packet), "ALERT|%.6f|%.6f", latitude, longitude);
  else
    snprintf(packet, sizeof(packet), "ALERT|NO_FIX");

  Serial.print("Sending ESP-NOW packet: ");
  Serial.println(packet);
  if (esp_now_send(gatewayMAC, (uint8_t*)packet, strlen(packet) + 1) != 0)
    Serial.println("ESP-NOW SEND REQUEST FAILED");
}

void OnDataSent(uint8_t* mac, uint8_t status) {
  Serial.println(status == 0 ? "ESP-NOW: PACKET SENT" : "ESP-NOW: SEND FAILED");
}

void updateClock() {
  time_t now = time(nullptr);
  if (now >= VALID_TIME) timeIsValid = true;
  if (!timeIsValid) {
    showMessage(WiFi.status() == WL_CONNECTED ? "WiFi connected" : "Clock waiting",
                WiFi.status() == WL_CONNECTED ? "Waiting for NTP..." : "Hotspot/NTP needed");
    return;
  }
  struct tm t;
  if (!localtime_r(&now, &t)) { showMessage("Clock error", "Time conversion failed"); return; }
  char tm[20], day[16], date[16];
  strftime(tm, sizeof(tm), "%I:%M:%S %p", &t);
  strftime(day, sizeof(day), "%A", &t);
  strftime(date, sizeof(date), "%d-%m-%Y", &t);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 8); display.println(tm);
  display.setCursor(0, 24); display.println(day);
  display.setCursor(0, 40); display.println(date);
  display.display();
}

void syncClockFromHotspot() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  bool connected = false;

  showMessage("Connecting WiFi", "Please wait...");
  for (int attempt = 1; attempt <= 3 && !connected; attempt++) {
    Serial.print("Hotspot attempt "); Serial.println(attempt);
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      readGPS(); delay(100); yield();
    }
    connected = WiFi.status() == WL_CONNECTED;
    if (!connected) { WiFi.disconnect(); delay(500); }
  }

  if (connected) {
    Serial.println("HOTSPOT CONNECTED");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    Serial.print("Hotspot channel: "); Serial.println(WiFi.channel());
    showMessage("WiFi connected", "Getting time...");
    configTime(timezone, "pool.ntp.org", "time.nist.gov");

    unsigned long start = millis();
    while (millis() - start < 30000) {
      readGPS();
      time_t now = time(nullptr);
      if (now >= VALID_TIME) {
        timeIsValid = true;
        Serial.println("TIME SYNC SUCCESSFUL");
        Serial.print("Unix time: "); Serial.println((long)now);
        break;
      }
      delay(250); yield();
    }
    if (!timeIsValid) {
      Serial.println("NTP FAILED: hotspot connected but time not received");
      showMessage("NTP failed", "Check hotspot internet/DNS");
      delay(2000);
    }
  } else {
    Serial.println("HOTSPOT CONNECTION FAILED");
    showMessage("WiFi failed", "Check 2.4GHz/SSID");
    delay(2000);
  }

  WiFi.disconnect();
  delay(200);
  wifi_set_channel(WIFI_CHANNEL);
  Serial.println("Radio set to ESP-NOW channel 1");
}

void startVibration() {
  digitalWrite(VIBRATION_PIN, HIGH);
  vibrationActive = true;
  vibrationStart = millis();
}
void updateVibration() {
  if (vibrationActive && millis() - vibrationStart >= vibrationDuration) {
    digitalWrite(VIBRATION_PIN, LOW);
    vibrationActive = false;
  }
}
void lightOn() {
  pixels.setBrightness(80);
  for (int i = 0; i < NUM_LEDS; i++) pixels.setPixelColor(i, pixels.Color(255,255,255));
  pixels.show(); emergencyLightActive = true;
}
void lightOff() { pixels.clear(); pixels.show(); emergencyLightActive = false; }

void handleSOSButton() {
  bool reading = digitalRead(SOS_BUTTON);
  if (reading != lastButtonReading) lastDebounceTime = millis();

  if (millis() - lastDebounceTime >= debounceTime && reading != stableButtonState) {
    stableButtonState = reading;
    if (stableButtonState == LOW) {
      unsigned long now = millis();
      if (firstPressTime != 0 && now - firstPressTime <= doublePressWindow && !doublePressHandled) {
        doublePressHandled = true;
        if (emergencyLightActive) lightOff(); else lightOn();
        Serial.println("DOUBLE PRESS: LED TOGGLED");
      } else {
        firstPressTime = now;
        doublePressHandled = false;
        Serial.println("SOS BUTTON PRESSED");
        sendESPNowAlert();
        startVibration();
      }
    }
  }
  lastButtonReading = reading;
  if (firstPressTime != 0 && millis() - firstPressTime > doublePressWindow) {
    firstPressTime = 0; doublePressHandled = false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  gpsSerial.begin(9600);
  Serial.println("GPS initialized: TX -> D3");

  Wire.begin(12, 5); // SDA D6, SCL D1
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED initialization failed");
    while (true) delay(1000);
  }
  showMessage("SHAKTIBANDH", "Starting...");

  pinMode(SOS_BUTTON, INPUT_PULLUP);
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);
  pixels.begin(); pixels.clear(); pixels.setBrightness(80); pixels.show();

  syncClockFromHotspot();

  Serial.print("Wearable MAC: "); Serial.println(WiFi.macAddress());
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW INIT FAILED");
    showMessage("ESP-NOW failed", "Check Serial Monitor");
    while (true) delay(1000);
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  if (esp_now_add_peer(gatewayMAC, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0) == 0)
    Serial.println("Gateway peer added");
  else
    Serial.println("Gateway peer add FAILED");

  Serial.println("SHAKTIBANDH READY");
  updateClock();
}

void loop() {
  readGPS();
  if (millis() - lastClockUpdate >= 250) {
    lastClockUpdate = millis();
    updateClock();
  }
  updateVibration();
  handleSOSButton();
  yield();
}
