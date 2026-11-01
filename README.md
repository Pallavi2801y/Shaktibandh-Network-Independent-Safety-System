

# Shaktibandh – Network-Independent Personal Safety and Emergency Alert System

**An IoT-based personal safety and emergency alert system featuring direct network-independent communication, GPS-based location tracking, visual-distraction defence, and a shared GSM gateway for multi-user emergency notifications.**

---

## 1. Project Overview

Shaktibandh is an IoT-based personal safety and emergency alert system designed to help individuals request assistance during emergency situations.

The system consists of two main units: wearable transmitter devices carried by users and a common gateway receiver responsible for processing emergency alerts and notifying designated contacts, including parents, emergency contacts, and authorities.

When a wearer activates the SOS button, the wearable transmitter sends an emergency alert and available GPS coordinates to the gateway using ESP-NOW. The gateway receives this information, activates a local buzzer, and uses its GSM module to send SMS notifications containing emergency information, GPS coordinates, and a Google Maps location link.

**A major advantage of Shaktibandh is its network-independent communication architecture and shared-gateway design.** Wearable transmitters communicate directly with the common gateway without depending on any external communication network. No Wi-Fi network, internet connection, cellular network, or wireless router is required for this local communication.

Furthermore, GSM hardware is not required in every wearable device. A single GSM module installed at the common gateway can provide SMS notification services for multiple connected wearable users. This reduces the hardware cost, size, and power requirements of individual wearables.

An additional defensive feature is an 8×8 WS2812B LED matrix designed to create a sudden visual distraction. The intention is to provide the wearer with a brief opportunity to escape, move to a safer location, or hide while help is being arranged.

## 2. Objectives

- Provide a convenient SOS mechanism for emergency situations.
- Establish direct wireless communication between wearable devices and a common gateway without relying on an external communication network.
- Support a shared-gateway architecture for multiple wearable users.
- Eliminate the need for individual GSM modules and SIM cards in every wearable.
- Obtain the wearer's geographical location using GPS.
- Notify designated emergency contacts through GSM-based SMS.
- Provide an active visual-distraction feature as an additional defensive measure.
- Provide audible and tactile feedback during emergency situations.

## 3. Key Features

- **SOS Emergency Activation:** Manual emergency triggering through a push button.
- **Direct Network-Independent Communication:** ESP-NOW communication between wearable transmitters and a common gateway without requiring an external communication network.
- **Shared GSM Gateway:** A single GSM module at the gateway can provide SMS notifications for multiple connected wearable users.
- **GPS Location Tracking:** NEO-6M GPS module for obtaining geographical coordinates independently of internet connectivity.
- **GSM Emergency Notifications:** SIM900A module at the gateway for sending SMS alerts to configured emergency contacts.
- **Google Maps Location Sharing:** GPS coordinates are converted into a location link that recipients can open using Google Maps.
- **Visual-Distraction Defence:** 8×8 WS2812B LED matrix designed to produce sudden bright illumination to distract an attacker temporarily.
- **Vibration Feedback:** Vibration motor module for tactile feedback.
- **OLED Display:** Displays relevant system information, including time and date.
- **Local Gateway Alert:** Buzzer provides an audible indication when an emergency alert is received.

## 4. System Architecture

Shaktibandh is designed around two primary hardware units: wearable transmitter devices and a common gateway receiver.

### 4.1 Wearable Transmitter Unit

Each wearable transmitter is built around the ESP8266 NodeMCU. It is responsible for detecting SOS activation, obtaining GPS information, providing local defensive and feedback features, and transmitting emergency information directly to the gateway.

| Component | Purpose |
|---|---|
| NodeMCU ESP8266 | Main controller |
| SOS push button | Manual emergency activation |
| NEO-6M GPS module | Obtains geographical coordinates |
| WS2812B 8×8 LED matrix | Visual-distraction defence mechanism |
| OLED SSD1306 display | Displays system information |
| Vibration motor module | Tactile feedback |

Each wearable communicates with the common gateway using ESP-NOW. It does not need its own GSM module, SIM card, Wi-Fi router, or internet connection to transmit an emergency alert to the gateway.

### 4.2 Common Gateway Receiver Unit

The common gateway is also based on the ESP8266 NodeMCU. It receives emergency information from connected wearable transmitters and manages local and external notifications.

| Component | Purpose |
|---|---|
| NodeMCU ESP8266 | Gateway controller |
| SIM900A GSM module | Sends emergency SMS notifications |
| Buzzer | Provides local audible alerts |

The gateway acts as a shared communication point. Multiple wearable users can connect to the same gateway and use its GSM communication facility.

This architecture avoids installing a separate GSM module and SIM card in every wearable. The number of wearable devices supported depends on gateway firmware, ESP-NOW peer capacity, communication range, and message-handling requirements.

## 5. Visual-Distraction Defence Mechanism

An important feature of Shaktibandh is the integration of an 8×8 WS2812B LED matrix as an active visual-distraction mechanism.

The concept is inspired by the temporary visual disturbance that may occur when a person is suddenly exposed to bright illumination.

During an emergency, the LED matrix is intended to produce a sudden bright light pattern to momentarily distract an attacker. This may give the wearer additional time to escape, seek shelter, or hide until assistance arrives.

**Intended functions:**

- Create sudden visual distraction.
- Potentially interrupt an attacker's attention momentarily.
- Provide the wearer with an additional opportunity to escape or reach safety.

This feature is intended as an additional defensive aid, not a guaranteed method of incapacitating an attacker. Its effectiveness and duration require controlled testing.

## 6. Communication Architecture

The communication process consists of two distinct stages. The first stage operates independently of external communication networks, while the second stage uses cellular communication to notify people outside the local system.

### 6.1 Wearable-to-Gateway Communication

- **Technology:** ESP-NOW.
- Multiple wearable transmitters can communicate directly with a common gateway.
- Communication takes place directly between ESP8266 devices.
- No external communication network is required.
- No Wi-Fi router, internet service, or cellular network is needed for this communication.
- The gateway receives SOS information and available GPS coordinates from wearable devices.

This means the wearable-to-gateway communication link can operate even when external network services are unavailable, provided the wearable and gateway are powered, within communication range, and functioning correctly.

### 6.2 Gateway-to-Emergency Contacts

- **Technology:** GSM using the SIM900A module.
- The common gateway uses the cellular network to send SMS notifications to configured emergency contacts.
- The same gateway GSM module can serve multiple connected wearable users.
- Messages can contain SOS information, GPS coordinates, and a Google Maps link.
- External SMS delivery requires cellular network coverage and an active SIM.

Thus, GSM is needed only at the common gateway for external notifications, not in every wearable transmitter.

### 6.3 GPS Positioning

The NEO-6M GPS module obtains geographical coordinates using satellite signals independently of internet or cellular network connectivity.

The coordinates can be used to generate a Google Maps location link. Opening the map link generally requires internet connectivity on the recipient's device.

### 6.4 Shared-Gateway Advantage

A major design advantage of Shaktibandh is the separation of local emergency communication from external notification services.

Instead of equipping every wearable with a GSM module, the system uses one common GSM-enabled gateway to serve multiple connected users.

This provides several advantages:

- Reduces the hardware cost of each wearable.
- Reduces wearable size and power requirements.
- Avoids the need for a separate SIM card in every wearable.
- Centralizes external SMS communication at the gateway.
- Allows multiple users to share one emergency notification facility.
- Provides an architecture that can be extended to support additional wearable users.

The gateway must be configured to identify the originating wearable and associate each emergency alert with the correct user and location. Its practical capacity depends on the wireless communication range, ESP-NOW peer limits, firmware design, and the number of simultaneous alerts.

## 7. Emergency Alert Workflow

1. The wearer presses the SOS push button.
2. The wearable activates its emergency response features.
3. The LED matrix provides the intended visual-distraction effect.
4. The GPS module supplies the available geographical coordinates.
5. The wearable transmitter sends the SOS message and location information directly to the common gateway using ESP-NOW.
6. The gateway receives and processes the emergency message.
7. The gateway activates the buzzer.
8. The gateway uses its SIM900A GSM module to send SMS notifications to the configured emergency contacts for the relevant user.
9. The recipient receives the emergency information and, when available, a Google Maps location link.

When multiple wearable devices are connected, the common gateway can handle emergency alerts from different users using the same GSM module, subject to its firmware and communication capacity.

If GPS coordinates are unavailable, the system can communicate the emergency alert without a valid location fix.

## 8. Hardware Connections

### 8.1 Wearable Transmitter – NodeMCU ESP8266

| Component | NodeMCU Pin |
|---|---|
| SOS push button | D5 (GPIO14) |
| WS2812B LED matrix DIN | D2 (GPIO4) |
| Vibration motor module IN | D7 (GPIO13) |
| OLED SDA | D6 (GPIO12) |
| OLED SCL | D1 (GPIO5) |
| NEO-6M GPS TX | D3 (GPIO0) |

The SOS button uses the internal pull-up configuration, with its other terminal connected to GND.

### 8.2 Common Gateway Receiver – NodeMCU ESP8266

| Component | NodeMCU Pin |
|---|---|
| Buzzer | D7 (GPIO13) |
| SIM900A TX | D1 (GPIO5) |
| SIM900A RX | D6 (GPIO12) |

**Power considerations:** The SIM900A requires a suitable separate power supply. The gateway and GSM module must share a common ground.

## 9. Software and Technologies

- Arduino IDE
- ESP8266 NodeMCU
- ESP-NOW direct wireless communication
- NEO-6M GPS module
- SIM900A GSM module
- UART and SoftwareSerial
- OLED SSD1306
- WS2812B LED matrix
- Embedded C/C++ (Arduino framework)

## 10. Testing and Results

The completed Shaktibandh prototype was tested for its major communication, positioning, and emergency notification functions.

| Test | Observed Result |
|---|---|
| GPS location acquisition | Successful |
| ESP-NOW communication | Successful |
| Gateway reception of GPS coordinates | Successful |
| SIM900A GSM communication | Successful |
| Delivery of three separate SMS messages | Successful |
| Google Maps location verification | Successful |

During the final end-to-end test, all three SMS messages were successfully received, and the Google Maps link displayed the correct location.

These results demonstrate the tested prototype's communication and notification functions. Multi-user operation through a shared gateway requires additional testing with multiple wearable transmitters.

The visual-distraction feature is an intended defensive function. Its effectiveness against an attacker has not yet been established through controlled testing.

## 11. Project Limitations

- External SMS delivery depends on cellular network availability.
- GPS acquisition may take time and may be affected by indoor environments and poor satellite visibility.
- Direct wearable-to-gateway communication depends on the availability and range of the ESP-NOW wireless link.
- The number of wearable devices supported by one gateway depends on ESP-NOW peer capacity and gateway firmware.
- Simultaneous emergency alerts require appropriate message handling at the gateway.
- The visual-distraction mechanism is not a guaranteed defence and requires further controlled evaluation.
- The current prototype uses manual SOS activation.
- The prototype is not a replacement for certified emergency response equipment or official emergency services.

## 12. Future Scope

Potential improvements include:

- Miniaturization of the wearable hardware.
- Improved battery capacity and power management.
- Further evaluation of direct wireless communication range.
- Multi-user and simultaneous-alert testing.
- Improved gateway firmware for identifying and managing multiple wearable users.
- Controlled testing of the visual-distraction mechanism.
- Improved wearable enclosure and ergonomics.
- Additional emergency notification options.
- Extended field testing in different environments.
- Further improvements in reliability and response time.

## 13. Repository Structure

```text
Shaktibandh-Network-Independent-Safety-System/
│
├── README.md
├── LICENSE
│
├── Code/
│   ├── Wearable_Transmitter/
│   │   └── transmitter.ino
│   │
│   └── GSM_Gateway/
│       └── gateway.ino
│
├── Hardware/
│   ├── Components.md
│   └── Circuit_Connections.md
│
├── Images/
│   ├── transmitter.jpg
│   ├── gateway.jpg
│   └── prototype.jpg
│
└── Documentation/
    └── Testing_and_Results.md
```

## 14. Research and Development

Shaktibandh was developed as an IoT-based personal safety prototype integrating direct wireless communication, GPS positioning, GSM-based emergency notifications, and an active visual-distraction feature.

The project explores a network-independent communication architecture in which multiple wearable devices communicate directly with a common gateway using ESP-NOW. A single GSM module installed at the gateway provides external emergency notifications through SMS.

This architecture separates local emergency communication from cellular notification services. It reduces the need for cellular hardware in individual wearables and offers a practical direction for multi-user personal safety systems.

The prototype demonstrates an integrated approach to emergency alerting and personal safety through embedded systems and wireless technologies.

## 15. Project Information

**Project Name:** Shaktibandh  
**Category:** Internet of Things (IoT), Embedded Systems, Personal Safety  
**Microcontroller:** ESP8266 NodeMCU  
**Wearable-to-Gateway Communication:** Direct ESP-NOW communication  
**External Notification:** GSM / SMS through a common gateway  
**Location Technology:** GPS  
**Architecture:** Multiple wearable transmitters with a shared GSM-enabled gateway

---

*Shaktibandh – A connected approach to personal safety, emergency awareness, and timely assistance.*

