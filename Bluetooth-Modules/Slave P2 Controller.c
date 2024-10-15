#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Bounce2.h>

// Revised UUIDs (Ensure these are unique and match the master device)
#define SERVICE_UUID        "00bf47c4-f07e-48d3-819f-4beafd84477c"
#define CHARACTERISTIC_UUID "3239a287-250a-40b8-92a7-8af2b3239bb7"

// Debounce interval in milliseconds
#define BOUNCE_INT 25

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;

// Callback class to handle connection events
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Client Connected");
  };
  
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Client Disconnected");
    // Restart advertising
    pServer->getAdvertising()->start();
    Serial.println("Restarted Advertising");
  }
};

// Initialize BLE
void setupBLE() {
  Serial.println("Initializing BLE...");
  BLEDevice::init("GameController_P2");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  // Get the BLEAdvertising object
  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  // Add the service UUID to the advertising packet
  pAdvertising->addServiceUUID(SERVICE_UUID);

  // Adjust the advertisement interval
  pAdvertising->setMinInterval(400);  // Minimum interval (in 0.625ms units, 400 * 0.625ms = 250ms)
  pAdvertising->setMaxInterval(500);  // Maximum interval (in 0.625ms units, 500 * 0.625ms = 312.5ms)

  pAdvertising->start();
  Serial.println("BLE Advertising Started with modified intervals");
}

// Define button pins (Revised to include the 11th button: PAUSE)
const uint8_t BUTTON_PINS[11] = {13, 11, 16, 12, 1, 3, 2, 10, 44, 43, 21};

// Array indices for readability
enum Buttons {LEFT, RIGHT, UP, DOWN, X, Y, A, B, M, P, PAUSE};

// Button names for display (Ensure this array has exactly 11 elements)
const char* buttonNames[11] = {"LEFT", "RIGHT", "UP", "DOWN", "X", "Y", "A", "B", "M", "P", "PAUSE"};

// Initialize button pins
void setupPins() {
  for (int i = 0; i < 11; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP); // Assuming buttons connect to GND when pressed
    Serial.print("Button ");
    Serial.print(i + 1); // Start counting from 1 for readability
    Serial.print(" (");
    Serial.print(buttonNames[i]);
    Serial.print(") set to GPIO ");
    Serial.println(BUTTON_PINS[i]);
  }
}

// Initialize debouncers
Bounce * buttonDebouncers = new Bounce[11];

void setupDebouncers() {
  for (int i = 0; i < 11; i++) {
    buttonDebouncers[i].attach(BUTTON_PINS[i]);
    buttonDebouncers[i].interval(BOUNCE_INT); // Debounce interval in milliseconds
    Serial.print("Debouncer set for GPIO ");
    Serial.println(BUTTON_PINS[i]);
  }
}

// Current and previous button states
bool currentButtonStates[11] = {0};
bool previousButtonStates[11] = {0};

// Function to check button states
bool checkButtonStates() {
  bool stateChanged = false;
  for (int i = 0; i < 11; i++) {
    buttonDebouncers[i].update();
    currentButtonStates[i] = !buttonDebouncers[i].read(); // Invert if using INPUT_PULLUP
    if (currentButtonStates[i] != previousButtonStates[i]) {
      stateChanged = true;
    }
  }
  return stateChanged;
}

// Variables to hold packed button data
uint8_t buttonData = 0;
uint8_t buttonData2 = 0;

// Function to pack button data
void packButtonData() {
  buttonData = 0;
  for (int i = 0; i < 8; i++) { // First 8 buttons: LEFT, RIGHT, UP, DOWN, X, Y, A, B
    buttonData |= (currentButtonStates[i] << i);
  }
}

void packButtonData2() {
  buttonData2 = 0;
  for (int i = 8; i < 11; i++) { // Next 3 buttons: M, P, PAUSE
    buttonData2 |= (currentButtonStates[i] << (i - 8));
  }
}

// Function to send data over BLE
void sendData() {
  if (deviceConnected) {
    uint8_t dataToSend[2] = {buttonData, buttonData2};
    pCharacteristic->setValue(dataToSend, 2);
    pCharacteristic->notify();
    Serial.print("Data Sent: ");
    Serial.print("0x");
    Serial.print(buttonData, HEX);
    Serial.print(" 0x");
    Serial.println(buttonData2, HEX);
  }
}

// Function to reliably send data with retries
void reliableSendData() {
  const int maxRetries = 3;
  int retryCount = 0;
  bool success = false;

  while (retryCount < maxRetries && !success) {
    if (deviceConnected) {
      sendData();
      success = true; // Assume success if device is connected
    } else {
      retryCount++;
      Serial.println("Send failed, retrying...");
      delay(100); // Wait before retrying
    }
  }

  if (!success) {
    Serial.println("Failed to send data after retries");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to initialize
  Serial.println("Starting Game Controller...");

  setupPins();
  setupDebouncers();
  setupBLE();
}

void loop() {
  bool stateChanged = checkButtonStates();

  if (stateChanged) {
    packButtonData();
    packButtonData2();
    reliableSendData();

    // Update previous states
    for (int i = 0; i < 11; i++) {
      previousButtonStates[i] = currentButtonStates[i];
    }
  }

  delay(10); // Short delay to prevent excessive CPU usage
}
