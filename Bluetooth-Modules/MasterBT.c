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

// Constants
#define SCAN_TIME          10      // Seconds to scan for controllers
#define MAX_CONTROLLERS    2       // Maximum number of controllers to connect
#define CONNECT_TIMEOUT    10      // Connection timeout in seconds

// UUIDs (must match the controller's SERVICE_UUID and CHARACTERISTIC_UUID)
#define SERVICE_UUID        "00bf47c4-f07e-48d3-819f-4beafd84477c"
#define CHARACTERISTIC_UUID "3239a287-250a-40b8-92a7-8af2b3239bb7"

// Struct to hold controller information
struct Controller {
  int id;                                // Unique ID assigned by the master
  BLEAddress address;                    // BLE Address of the device
  BLEClient* client;                     // BLE Client instance
  BLERemoteService* service;             // Remote Service
  BLERemoteCharacteristic* characteristic; // Remote Characteristic
  bool buttons[10];                      // Latest button states

  // Constructor
  Controller(BLEAddress addr) 
    : address(addr), client(nullptr), service(nullptr), characteristic(nullptr), id(0) {
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

// Forward declaration of MyClientCallback
class MyClientCallback;

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
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      std::string mac = advertisedDevice.getAddress().toString();

      // Check if already connected and if maximum controllers reached
      if (connectedControllers.find(mac) == connectedControllers.end() && connectedControllers.size() < MAX_CONTROLLERS) {
        Serial.print("Desired controller found with MAC: ");
        Serial.println(mac.c_str());

        // Add to connection queue
        connectionQueue.push_back(advertisedDevice.getAddress());
        Serial.println("Controller added to connection queue.");
      }
    }
  }
};

// Callback class to handle BLE client connection events (Final)
class MyClientCallbacksFinal : public BLEClientCallbacks {
public:
  void onConnect(BLEClient* pclient) override {
    Serial.println("Final Client Connected.");
  }

  void onDisconnect(BLEClient* pclient) override {
    Serial.println("Final Client Disconnected.");
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Master ESP32 Starting...");

  // Initialize BLE
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

void loop() {
  // Handle connection queue
  if (!isConnecting && !connectionQueue.empty() && connectedControllers.size() < MAX_CONTROLLERS) {
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
            newController->characteristic->registerForNotify([newController](BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
              if (length >= 2) { // Expecting at least 2 bytes
                uint8_t buttonData = pData[0];
                uint8_t buttonData2 = pData[1];

                // Decode button states
                for (int i = 0; i < 8; i++) {
                  newController->buttons[i] = buttonData & (1 << i);
                }
                newController->buttons[8] = buttonData2 & 0x01;        // BACK
                newController->buttons[9] = (buttonData2 & 0x02) >> 1; // START

                // Print button states
                Serial.print("Controller ");
                Serial.print(newController->id);
                Serial.println(" Button States:");
                const char* buttonNames[10] = {"UP", "DOWN", "LEFT", "RIGHT", "A", "B", "X", "Y", "BACK", "START"};
                for (int i = 0; i < 10; i++) {
                  Serial.print("  ");
                  Serial.print(buttonNames[i]);
                  Serial.print(": ");
                  Serial.println(newController->buttons[i] ? "Pressed" : "Released");
                }
                Serial.println();
              } else {
                Serial.println("Received insufficient data length for button states.");
              }
            });

            Serial.print("Notifications subscribed for Controller ");
            Serial.println(newController->id);

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
