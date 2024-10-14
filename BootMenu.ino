#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLE2902.h>
#include <map>
#include <vector>
#include <string>
#include <TFT_eSPI.h>
#include "Tetris.h"
#include "Pong.h"
#include "Snake.h"
#include "Chess.h"
#include "Buttons.h" // Include Buttons.h

// Initialize TFT object
TFT_eSPI tft = TFT_eSPI();

// Constants
#define SCAN_TIME          10      // Seconds to scan for controllers
#define MAX_CONTROLLERS    2       // Maximum number of controllers to connect
#define CONNECT_TIMEOUT    10      // Connection timeout in seconds

// UUIDs (must match the controller's SERVICE_UUID and CHARACTERISTIC_UUID)
#define SERVICE_UUID        "00bf47c4-f07e-48d3-819f-4beafd84477c"
#define CHARACTERISTIC_UUID "3239a287-250a-40b8-92a7-8af2b3239bb7"

// Button-to-GPIO pin assignments (Defined as const)
const int leftButton   = 13;
const int rightButton  = 11;
const int upButton     = 16;
const int downButton   = 12;

const int xButton      = 1;
const int yButton      = 3;

const int aButton      = 2;
const int bButton      = 10;

const int mButton      = 44;
const int pButton      = 43;

const int pauseButton  = 21;
bool paused = false;

// Button names for display (Ensure this array has exactly 11 elements)
const char* buttonNames[11] = {
  "LEFT", "RIGHT", "UP", "DOWN", "X", "Y", "A", "B", "M", "P", "PAUSE"
};

// Menu variables
String games[] = {"Tetris", "Pong", "Snake", "Chess"};
int currSelect = 0;
int totalGames = sizeof(games) / sizeof(games[0]);

// Struct to hold controller information
struct Controller {
  int id;                                      // Unique ID assigned by the master
  BLEAddress address;                          // BLE Address of the device
  BLEClient* client;                           // BLE Client instance
  BLERemoteService* service;                   // Remote Service
  BLERemoteCharacteristic* characteristic;     // Remote Characteristic
  bool buttons[11];                            // Latest button states

  // Constructor
  Controller(BLEAddress addr)
    : id(0), address(addr), client(nullptr),
      service(nullptr), characteristic(nullptr) {
    memset(buttons, 0, sizeof(buttons));
  }

  // Destructor to clean up resources
  ~Controller() {
    if (client) {
      client->disconnect();
      delete client;
    }
    if (service) {
      delete service;
    }
    if (characteristic) {
      delete characteristic;
    }
  }
};

// Global variables
BLEScan* pBLEScan;
std::map<std::string, Controller*> connectedControllers; // Map of MAC Address to Controller*
int nextControllerID = 1;                                // Counter for assigning unique IDs
bool isConnecting = false;                               // Flag to indicate if a connection attempt is in progress
std::vector<BLEAddress> connectionQueue;                 // Queue of devices to connect to

// ButtonStates struct to track current and previous states
struct ButtonStates {
  bool current[11];
  bool previous[11];
};

std::map<int, ButtonStates> controllerButtonStates; // Map of controller ID to their button states

// Callback class to handle BLE client connection events
class MyClientCallback : public BLEClientCallbacks {
public:
  Controller* controller;

  MyClientCallback(Controller* ctrl) : controller(ctrl) {}

  void onConnect(BLEClient* pclient) override {
    // Optional: Add actions upon successful connection
  }

  void onDisconnect(BLEClient* pclient) override {
    if (controller) {
      std::string mac = controller->address.toString();
      connectedControllers.erase(mac);
      delete controller;
      controller = nullptr;
    }
    isConnecting = false;
    Serial.println("Controller disconnected.");
  }
};

// Callback class to handle advertised devices during scanning
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
public:
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    // Check if the device advertises the desired service UUID
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      std::string mac = advertisedDevice.getAddress().toString();

      // Check if already connected and if maximum controllers reached
      if (connectedControllers.find(mac) == connectedControllers.end() &&
          connectedControllers.size() < MAX_CONTROLLERS) {
        Serial.print("Desired controller found with MAC: ");
        Serial.println(mac.c_str());

        // Add to connection queue
        connectionQueue.push_back(advertisedDevice.getAddress());
        Serial.println("Controller added to connection queue.");
      }
    }
  }
};

// Function to initialize BLE
void setupBLE() {
  Serial.println("Initializing BLE...");
  BLEDevice::init("");
  Serial.println("BLE initialized.");

  // Create BLE scan instance
  pBLEScan = BLEDevice::getScan(); // Create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);    // Active scan to get more data
  pBLEScan->setInterval(500);       // Scan interval in milliseconds
  pBLEScan->setWindow(499);         // Scan window in milliseconds (must be <= interval)
  Serial.println("BLE Scan instance created and configured.");

  // Start initial scanning
  Serial.println("Starting scan for controllers...");
  pBLEScan->start(SCAN_TIME, false);
  Serial.println("Scan complete.");
}

// Function to draw the menu
void drawMenu() {
  // Initialize screen as black
  tft.fillScreen(TFT_BLACK);
  for (int i = 0; i < totalGames; i++) {
    if (i == currSelect) {
      tft.setTextColor(TFT_RED); // If currently selected, highlight as red
      tft.drawString("->", 5, 20 + i * 20);
    } else {
      tft.setTextColor(TFT_WHITE); // Otherwise white
    }
    tft.drawString(games[i], 35, 20 + i * 20);
  }
}

// Function to launch games
void launchTetris() {
  tft.fillScreen(TFT_BLACK);
  tetrisSetup();
  while (true) {
    tetrisLoop();
  }
}

void launchPong() {
  tft.fillScreen(TFT_BLACK);
  pongSetup();
  while (true) {
    pongLoop();
  }
}

void launchSnake() {
  tft.fillScreen(TFT_BLACK);
  snakeSetup();
  while (true) {
    snakeLoop();
  }
}

void launchChess() {
  tft.fillScreen(TFT_BLACK);
  chessSetup();
  while (true) {
    if (isPauseButtonPressed()) {
      break; // Return to menu when pause button is pressed
    }
    chessLoop();
  }
  drawMenu();
}

// Function to check if pause button is pressed (from controller inputs)
bool isPauseButtonPressed() {
  // Check if any connected controller has the PAUSE button pressed
  for (const auto& pair : connectedControllers) {
    Controller* controller = pair.second;
    if (controller->buttons[10]) { // PAUSE button is index 10
      return true;
    }
  }
  return false;
}

// Function to handle button actions from controllers
void handleButtonPress(int controllerID, int buttonIndex, bool pressed) {
  // Only act on button press (not release)
  if (!pressed) return;

  // Map buttons to menu actions
  switch (buttonIndex) {
    case 6: // A button
      Serial.printf("Controller %d: A button pressed. Launching %s...\n",
                    controllerID, games[currSelect].c_str());
      if (currSelect == 0) {
        launchTetris();
      } else if (currSelect == 1) {
        launchPong();
      } else if (currSelect == 2) {
        launchSnake();
      } else if (currSelect == 3) {
        launchChess();
      }
      break;
    case 2: // UP button
      Serial.printf("Controller %d: UP button pressed. Moving selection up.\n",
                    controllerID);
      currSelect = (currSelect - 1 + totalGames) % totalGames; // Wrap around to last game
      drawMenu();
      break;
    case 3: // DOWN button
      Serial.printf("Controller %d: DOWN button pressed. Moving selection down.\n",
                    controllerID);
      currSelect = (currSelect + 1) % totalGames; // Wrap around to first game
      drawMenu();
      break;
    case 7: // B button
      Serial.printf("Controller %d: B button pressed. No action defined.\n",
                    controllerID);
      // Optional: Implement B action if needed
      break;
    case 10: // PAUSE button
      Serial.printf("Controller %d: PAUSE button pressed. Exiting menu.\n",
                    controllerID);
      // Implement exiting the menu or other actions
      // For example, restarting the menu
      drawMenu();
      break;
    // Add cases for other buttons if needed
    default:
      break;
  }
}

// Function to initialize the Boot Menu and BLE
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to initialize
  Serial.println("Starting Boot Menu...");

  // Initialize TFT display
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(4); // Upside down for convenience, change this as needed
  tft.setTextSize(2);

  // Initialize menu
  drawMenu();

  // Initialize BLE
  setupBLE();
}

void loop() {
  // Handle connection queue
  if (!isConnecting && !connectionQueue.empty() &&
      connectedControllers.size() < MAX_CONTROLLERS) {
    BLEAddress address = connectionQueue.front();
    connectionQueue.erase(connectionQueue.begin());

    std::string mac = address.toString();
    if (connectedControllers.find(mac) != connectedControllers.end()) {
      // Already connected
      return;
    }

    Serial.print("Attempting to connect to controller with MAC: ");
    Serial.println(mac.c_str());

    // Create a new Controller instance
    Controller* newController = new Controller(address);
    newController->id = nextControllerID++;

    // Create BLE Client and set callbacks
    newController->client = BLEDevice::createClient();
    if (!newController->client) {
      Serial.println("Failed to create BLE client.");
      delete newController;
      return;
    }
    newController->client->setClientCallbacks(new MyClientCallback(newController));

    Serial.println("Attempting to connect to Controller...");
    isConnecting = true;

    // Attempt to connect with timeout
    unsigned long startAttemptTime = millis();
    bool connected = false;
    while ((millis() - startAttemptTime) < (CONNECT_TIMEOUT * 1000)) { // CONNECT_TIMEOUT in seconds
      connected = newController->client->connect(address);
      if (connected) {
        Serial.println("Connected to Controller.");
        break;
      } else {
        Serial.println("Connection attempt failed. Retrying...");
        delay(1000); // Wait 1 second before retrying
      }
    }

    if (connected) {
      // Obtain the service
      newController->service = newController->client->getService(SERVICE_UUID);
      if (newController->service != nullptr) {
        // Obtain the characteristic
        newController->characteristic = newController->service->getCharacteristic(CHARACTERISTIC_UUID);
        if (newController->characteristic != nullptr) {
          if (newController->characteristic->canNotify()) {
            // Register for notifications
            newController->characteristic->registerForNotify(
              [newController](BLERemoteCharacteristic* pBLERemoteCharacteristic,
                              uint8_t* pData, size_t length, bool isNotify) {
                if (length >= 2) { // Expecting at least 2 bytes
                  uint8_t buttonData = pData[0];
                  uint8_t buttonData2 = pData[1];

                  // Decode button states
                  for (int i = 0; i < 8; i++) {
                    newController->buttons[i] = (buttonData & (1 << i)) != 0;
                  }
                  newController->buttons[8]  = (buttonData2 & 0x01) != 0; // M
                  newController->buttons[9]  = (buttonData2 & 0x02) != 0; // P
                  newController->buttons[10] = (buttonData2 & 0x04) != 0; // PAUSE

                  // Handle button presses
                  for (int i = 0; i < 11; i++) {
                    if (newController->buttons[i] &&
                        !controllerButtonStates[newController->id].previous[i]) {
                      handleButtonPress(newController->id, i, true);
                    }
                    controllerButtonStates[newController->id].previous[i] = newController->buttons[i];
                  }
                } else {
                  Serial.println("Received insufficient data length for button states.");
                }
              });

            Serial.print("Notifications subscribed for Controller ");
            Serial.println(newController->id);

            // Initialize button states for this controller
            ButtonStates bs;
            for (int i = 0; i < 11; i++) {
              bs.current[i] = false;
              bs.previous[i] = false;
            }
            controllerButtonStates[newController->id] = bs;

            // Add to the connectedControllers map
            connectedControllers[mac] = newController;
          } else {
            Serial.println("Characteristic cannot notify.");
            newController->client->disconnect();
            delete newController;
          }
        } else {
          Serial.println("Failed to find characteristic.");
          newController->client->disconnect();
          delete newController;
        }
      } else {
        Serial.println("Failed to find service.");
        newController->client->disconnect();
        delete newController;
      }
    } else {
      Serial.println("Failed to connect to controller within timeout.");
      delete newController;
    }
    isConnecting = false;
  }

  // Rescan for controllers if needed
  if (connectedControllers.size() < MAX_CONTROLLERS) {
    Serial.println("Rescanning for controllers...");
    pBLEScan->start(SCAN_TIME, false);
    Serial.println("Rescan complete.");
  }

  // Delay between scans
  delay(10000); // 10 seconds
}
