// ControllerInput.cpp

#include "ControllerInput.h"

// Controller 1 button states
int leftButton = 1;
int rightButton = 1;
int upButton = 1;
int downButton = 1;
int xButton = 1;
int yButton = 1;
int aButton = 1;
int bButton = 1;
int mButton = 1;
int pButton = 1;
int pauseButton = 1;

// Controller 2 button states
int leftButton2 = 1;
int rightButton2 = 1;
int upButton2 = 1;
int downButton2 = 1;
int xButton2 = 1;
int yButton2 = 1;
int aButton2 = 1;
int bButton2 = 1;
int mButton2 = 1;
int pButton2 = 1;
int pauseButton2 = 1;

// Controller MAC addresses
uint8_t controller1MAC[6] = {0};
uint8_t controller2MAC[6] = {0};

// Button names and their indices
const char* buttonNames[11] = {"LEFT", "RIGHT", "UP", "DOWN", "X", "Y", "A", "B", "M", "P", "PAUSE"};

// Receive callback function
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  uint16_t receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  int controllerNumber = 0; // 1 or 2

  // Identify controller based on MAC address
  if (memcmp(mac, controller1MAC, 6) == 0) {
    controllerNumber = 1;
  } else if (memcmp(mac, controller2MAC, 6) == 0) {
    controllerNumber = 2;
  } else {
    // Assign new controller if slots are available
    if (controller1MAC[0] == 0) {
      memcpy(controller1MAC, mac, 6);
      controllerNumber = 1;
      Serial.print("Assigned new controller as Controller 1 with MAC: ");
        } else if (controller2MAC[0] == 0) {
      memcpy(controller2MAC, mac, 6);
      controllerNumber = 2;
      Serial.print("Assigned new controller as Controller 2 with MAC: ");

    } else {
      Serial.println("Received data from unknown device and no free controller slots.");
      return;
    }
  }

  // Update button states for the identified controller
  for (int i = 0; i < 11; i++) {
    int buttonState = (receivedData & (1 << i)) ? 1 : 0; // 0 for pressed, 1 for not pressed
    if (controllerNumber == 1) {
      switch (i) {
        case 0: leftButton = buttonState; break;
        case 1: rightButton = buttonState; break;
        case 2: upButton = buttonState; break;
        case 3: downButton = buttonState; break;
        case 4: xButton = buttonState; break;
        case 5: yButton = buttonState; break;
        case 6: aButton = buttonState; break;
        case 7: bButton = buttonState; break;
        case 8: mButton = buttonState; break;
        case 9: pButton = buttonState; break;
        case 10: pauseButton = buttonState; break;
      }
    } else if (controllerNumber == 2) {
      switch (i) {
        case 0: leftButton2 = buttonState; break;
        case 1: rightButton2 = buttonState; break;
        case 2: upButton2 = buttonState; break;
        case 3: downButton2 = buttonState; break;
        case 4: xButton2 = buttonState; break;
        case 5: yButton2 = buttonState; break;
        case 6: aButton2 = buttonState; break;
        case 7: bButton2 = buttonState; break;
        case 8: mButton2 = buttonState; break;
        case 9: pButton2 = buttonState; break;
        case 10: pauseButton2 = buttonState; break;
      }
    }
  }

  // For debugging: Print button states
  Serial.print("Controller ");
  Serial.print(controllerNumber);
  Serial.print(" button states updated: ");
  for (int i = 0; i < 11; i++) {
    int buttonState = (receivedData & (1 << i)) ? 1 : 0;
    if (buttonState == 0) { // Button is pressed
      Serial.print(buttonNames[i]);
      Serial.print(" ");
    }
  }
  Serial.println();
}

// Utility function to convert MAC address to string
String macToStr(const uint8_t *mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

// Initialize controller input
void initControllerInput() {
  // Initialize controller MAC addresses to zero
  memset(controller1MAC, 0, sizeof(controller1MAC));
  memset(controller2MAC, 0, sizeof(controller2MAC));

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("ESP-NOW initialized and receive callback registered.");
}

// Update controller input (if needed in loop)
void updateControllerInput() {
  // Currently, no periodic tasks are required
}
