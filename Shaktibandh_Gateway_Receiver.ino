#include <ESP8266WiFi.h>
#include <espnow.h>
#include <SoftwareSerial.h>
#include <math.h>

// SHAKTIBANDH GATEWAY - NodeMCU ESP8266
// Buzzer D7. SIM900A TX -> divider -> D1 (NodeMCU RX).
// NodeMCU D6 (TX) -> SIM900A RX. Common GND.
// SIM900A needs a suitable separate supply and must be powered on via PWRKEY.

#define BUZZER_PIN D7
#define GSM_RX D1
#define GSM_TX D6
#define WIFI_CHANNEL 1

const char PHONE_NUMBER[] = "+91XXXXXXXXXX"; // Replace with recipient number
SoftwareSerial gsm(GSM_RX, GSM_TX);

volatile bool alertReceived = false;
volatile bool receivedGPSFix = false;
volatile float receivedLatitude = 0.0f;
volatile float receivedLongitude = 0.0f;

unsigned long lastSMSTime = 0;
const unsigned long SMS_COOLDOWN = 5000;

bool waitForResponse(const char* expected, unsigned long timeout) {
  unsigned long start = millis();
  String response;
  while (millis() - start < timeout) {
    while (gsm.available()) {
      char c = (char)gsm.read();
      Serial.write(c);
      response += c;
      if (response.indexOf(expected) >= 0) return true;
      if (response.length() > 300) response.remove(0, 150);
    }
    yield();
  }
  return false;
}

bool sendAT(const char* command, const char* expected, unsigned long timeout) {
  while (gsm.available()) gsm.read();
  Serial.print("GSM >> "); Serial.println(command);
  gsm.println(command);
  return waitForResponse(expected, timeout);
}

// Run once at startup to identify why SMS submission may be rejected.
void diagnoseSMS() {
  Serial.println("\n===== SMS DIAGNOSTICS =====");
  sendAT("AT", "OK", 3000);
  sendAT("AT+CPIN?", "OK", 3000);
  sendAT("AT+CSQ", "OK", 3000);
  sendAT("AT+CREG?", "OK", 3000);
  sendAT("AT+CMGF=1", "OK", 3000);
  sendAT("AT+CSCA?", "OK", 3000);
  Serial.println("===== DIAGNOSTICS COMPLETE =====");
}

void OnDataRecv(uint8_t* mac, uint8_t* incomingData, uint8_t len) {
  char msg[64];
  if (len == 0 || len >= sizeof(msg)) {
    Serial.println("Invalid ESP-NOW packet length");
    return;
  }
  memcpy(msg, incomingData, len);
  msg[len] = '\0';
  Serial.print("ESP-NOW Received: "); Serial.println(msg);

  if (strcmp(msg, "ALERT|NO_FIX") == 0) {
    receivedGPSFix = false;
    alertReceived = true;
    Serial.println("SOS received, transmitter reports NO_FIX");
    return;
  }

  float lat, lon;
  if (sscanf(msg, "ALERT|%f|%f", &lat, &lon) == 2) {
    if (isfinite(lat) && isfinite(lon) &&
        lat >= -90.0f && lat <= 90.0f &&
        lon >= -180.0f && lon <= 180.0f) {
      receivedLatitude = lat;
      receivedLongitude = lon;
      receivedGPSFix = true;
      alertReceived = true;
      Serial.print("Latitude: "); Serial.println(lat, 6);
      Serial.print("Longitude: "); Serial.println(lon, 6);
    } else {
      Serial.println("Invalid GPS coordinate range");
    }
    return;
  }
  Serial.println("Unknown packet format");
}

bool initializeGSM() {
  Serial.println("Initializing SIM900A...");
  delay(8000);
  bool ok = false;

  for (int i = 1; i <= 5; i++) {
    Serial.print("AT test "); Serial.println(i);
    if (sendAT("AT", "OK", 2500)) { ok = true; break; }
    Serial.println("No AT response");
    delay(1000);
  }

  if (!ok) {
    Serial.println("SIM900A NOT RESPONDING. ESP-NOW will still start.");
    return false;
  }

  Serial.println("SIM900A AT communication OK");
  sendAT("ATE0", "OK", 3000);
  sendAT("AT+CPIN?", "OK", 3000);
  sendAT("AT+CSQ", "OK", 3000);
  sendAT("AT+CREG?", "OK", 3000);
  sendAT("AT+CMGF=1", "OK", 3000);
  return true;
}

bool sendSMS(const char* message) {
  Serial.println("===== SENDING SOS SMS =====");

  if (!sendAT("AT", "OK", 3000)) {
    Serial.println("SMS FAILED: SIM900A did not answer AT");
    return false;
  }
  if (!sendAT("AT+CMGF=1", "OK", 5000)) {
    Serial.println("SMS FAILED: could not set text mode");
    return false;
  }

  while (gsm.available()) gsm.read();
  gsm.print("AT+CMGS=\"");
  gsm.print(PHONE_NUMBER);
  gsm.println("\"");

  if (!waitForResponse(">", 10000)) {
    Serial.println("SMS FAILED: no CMGS prompt");
    return false;
  }

  gsm.print(message);
  delay(300);
  gsm.write(26); // Ctrl+Z

  unsigned long start = millis();
  String response;
  while (millis() - start < 30000) {
    while (gsm.available()) {
      char c = (char)gsm.read();
      Serial.write(c);
      response += c;
      if (response.indexOf("+CMGS:") >= 0) {
        Serial.println("SMS SENT: +CMGS confirmation received");
        return true;
      }
      if (response.indexOf("ERROR") >= 0) {
        Serial.println("Modem returned SMS ERROR");
        return false;
      }
      if (response.length() > 300) response.remove(0, 150);
    }
    yield();
  }
  Serial.println("SMS result timeout");
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  gsm.begin(9600);

  Serial.println();
  Serial.println("===== SHAKTIBANDH GATEWAY =====");
  initializeGSM();
  // Extra SMS diagnostics: SIM readiness, signal, network registration and SMSC.
  diagnoseSMS();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  wifi_set_channel(WIFI_CHANNEL);

  Serial.print("Gateway MAC: "); Serial.println(WiFi.macAddress());
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW INIT FAILED");
    while (true) delay(1000);
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW READY ON CHANNEL 1");
  Serial.println("GATEWAY READY");
}

void loop() {
  if (alertReceived && millis() - lastSMSTime >= SMS_COOLDOWN) {
    noInterrupts();
    bool gpsAvailable = receivedGPSFix;
    float lat = receivedLatitude;
    float lon = receivedLongitude;
    alertReceived = false;
    interrupts();

    Serial.println();
    Serial.println("!!!!!!!! SOS ALERT RECEIVED !!!!!!!!");

    char sms[250];
    if (gpsAvailable) {
      snprintf(sms, sizeof(sms),
        "SOS ALERT! Shaktibandh emergency button activated.\n"
        "Latitude: %.6f\nLongitude: %.6f\n"
        "Google Maps: https://maps.google.com/?q=%.6f,%.6f",
        lat, lon, lat, lon);
      Serial.println("SMS will include received coordinates");
    } else {
      snprintf(sms, sizeof(sms),
        "SOS ALERT! Shaktibandh emergency button activated.\n"
        "GPS LOCATION UNAVAILABLE. Wearable reported no recent fix.");
      Serial.println("GPS location unavailable");
    }

    digitalWrite(BUZZER_PIN, HIGH);
    bool sent = sendSMS(sms);
    digitalWrite(BUZZER_PIN, LOW);
    lastSMSTime = millis();
    Serial.println(sent ? "GATEWAY: SMS SENT" : "GATEWAY: SMS FAILED");
  }
  yield();
}
