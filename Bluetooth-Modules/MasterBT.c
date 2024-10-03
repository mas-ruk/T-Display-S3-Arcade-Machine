#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLE2902.h>
#include <map>
#include <string>

// Constants
#define SCAN_TIME 5          // Seconds to scan for controllers
#define MAX_CONTROLLERS 2    // Maximum number of controllers to connect

// UUIDs (must match the controller's SERVICE_UUID and CHARACTERISTIC_UUID)
#define SERVICE_UUID        "00bf47c4-f07e-48d3-819f-4beafd84477c"
#define CHARACTERISTIC_UUID "3239a287-250a-40b8-92a7-8af2b3239bb7"

// Struct to hold controller information
struct Controller {
  int id; // Unique ID assigned by the master
  BLEAdvertisedDevice device; // BLE Advertised Device
  BLEClient* client; // BLE Client instance
  BLERemoteService* service; // Remote Service
  BLERemoteCharacteristic* characteristic; // Remote Characteristic
  bool buttons[10]; // Latest button states
};

// Global variables
BLEScan* pBLEScan;
std::map<std::string, Controller*> connectedControllers; // Map of MAC Address to Controller*
int nextControllerID = 1; // Counter for assigning unique IDs

// Callback class to handle BLE client connection events
class MyClientCallback : public BLEClientCallbacks {
public:
  Controller* controller;

  MyClientCallback(Controller* ctrl) : controller(ctrl) {}

  void onConnect(BLEClient* pclient) override {
    Serial.print("Controller ");
    Serial.print(controller->id);
    Serial.println(" connected.");
  }

  void onDisconnect(BLEClient* pclient) override {
    Serial.print("Controller ");
    Serial.print(controller->id);
    Serial.println(" disconnected.");
    // Remove the controller from the map
    connectedControllers.erase(controller->device.getAddress().toString());
    delete controller->client;
    delete controller; // Free memory
    controller = nullptr;
  }
};

// Callback class to handle advertised devices during scanning
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    // Check if the advertised device has the desired service UUID
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      std::string mac = advertisedDevice.getAddress().toString();

      // Check if already connected and if maximum controllers reached
      if (connectedControllers.find(mac) == connectedControllers.end() && connectedControllers.size() < MAX_CONTROLLERS) {
        Serial.print("Found Controller ");
        Serial.print(nextControllerID);
        Serial.print(" with MAC: ");
        Serial.println(mac.c_str());

        // Dynamically allocate a new Controller instance
        Controller* newController = new Controller();
        newController->id = nextControllerID;
        newController->device = advertisedDevice;
        newController->client = BLEDevice::createClient();
        newController->client->setClientCallbacks(new MyClientCallback(newController));

        // Attempt to connect to the controller
        if (newController->client->connect(&advertisedDevice)) {
          Serial.print("Connected to Controller ");
          Serial.println(newController->id);

          // Obtain the service
          newController->service = newController->client->getService(SERVICE_UUID);
          if (newController->service != nullptr) {
            // Obtain the characteristic
            newController->characteristic = newController->service->getCharacteristic(CHARACTERISTIC_UUID);
            if (newController->characteristic != nullptr) {
              if (newController->characteristic->canNotify()) {
                // Register for notifications using a lambda function
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

                    // TODO: Integrate with your game's I/O system here
                    // Example:
                    // if (newController->buttons[0]) { movePlayerUp(newController->id); }
                    // if (newController->buttons[4]) { playerActionA(newController->id); }
                  }
                });

                Serial.print("Notifications registered for Controller ");
                Serial.println(newController->id);

                // Add to the connectedControllers map
                connectedControllers[mac] = newController;
                nextControllerID++;
              } else {
                Serial.println("Characteristic cannot notify.");
                newController->client->disconnect();
                delete newController; // Free memory
              }
            } else {
              Serial.println("Failed to find characteristic.");
              newController->client->disconnect();
              delete newController; // Free memory
            }
          } else {
            Serial.println("Failed to find service.");
            newController->client->disconnect();
            delete newController; // Free memory
          }
        } else {
          Serial.println("Failed to connect to controller.");
          delete newController; // Free memory
        }
      }
    }
  };
};

void setup() {
  Serial.begin(115200);
  Serial.println("Master ESP32 Starting...");

  // Initialize BLE
  BLEDevice::init("");

  // Create BLE scan instance
  pBLEScan = BLEDevice::getScan(); // Create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // Active scan to get more data
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // Less than or equal to setInterval value

  // Start initial scanning
  Serial.println("Scanning for controllers...");
  pBLEScan->start(SCAN_TIME, false);
  Serial.println("Initial scan complete.");
}

void loop() {
  // Periodically scan for new controllers if not at max
  if (connectedControllers.size() < MAX_CONTROLLERS) {
    Serial.println("Scanning for additional controllers...");
    pBLEScan->start(SCAN_TIME, false);
    Serial.println("Additional scan complete.");
  }

  // Handle game I/O
  // You can implement your game logic here or in a separate function
  // Example:
  // processGameInputs();

  // Delay between scans and I/O handling
  delay(10000); // 10 seconds
}
