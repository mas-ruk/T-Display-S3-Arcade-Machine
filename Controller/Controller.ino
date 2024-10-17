#include <esp_now.h>
#include <WiFi.h>

// Define button pins
const uint8_t BUTTON_PINS[11] = {13, 11, 16, 12, 1, 3, 2, 10, 44, 43, 21};
const char* buttonNames[11] = {"LEFT", "RIGHT", "UP", "DOWN", "X", "Y", "A", "B", "M", "P", "PAUSE"};

// Variables to hold button states
uint16_t currentButtonState = 0xFFFF; // All buttons not pressed (1)
uint16_t lastButtonState = 0xFFFF;    // Previous state for comparison

// Master device MAC address (replace with your master console's MAC address)
uint8_t masterMACAddress[] = {0x30, 0x30, 0xF9, 0x5A, 0x62, 0x94}; // Update XX with actual values

// Peer information
esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Fail");
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("\n--- ESP32 Controller Starting ---");

  // Set GPIO 15 HIGH to enable battery functionality
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  Serial.println("GPIO 15 set to HIGH to enable battery functionality.");

  // Initialize button pins as INPUT_PULLUP
  for (int i = 0; i < 11; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  Serial.println("Button pins initialized as INPUT_PULLUP.");

  // Initialize WiFi in station mode and disconnect from any network
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("WiFi initialized in STA mode and disconnected from any network.");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true) {
      // Halt execution if ESP-NOW initialization fails
      delay(1000);
    }
  }
  Serial.println("ESP-NOW initialized successfully.");

  // Register the send callback function
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, masterMACAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add master as peer
  Serial.print("Adding master console with MAC address: ");
  Serial.println(macToStr(masterMACAddress));

  if (esp_now_add_peer(&peerInfo) == ESP_OK){
    Serial.println("Master console added as a peer successfully. Connected.");
  } else {
    Serial.println("Failed to add master console as a peer.");
    while (true) {
      // Halt execution if peer addition fails
      delay(1000);
    }
  }

  Serial.println("--- Controller Setup Complete ---\n");
}

// Utility function to convert MAC address to string
String macToStr(const uint8_t *mac) {
  String macStr = "";
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 16) macStr += "0";
    macStr += String(mac[i], HEX);
    if (i < 5) macStr += ":";
  }
  return macStr;
}

void loop() {
  // Read all button states and update currentButtonState
  uint16_t newButtonState = 0xFFFF; // Reset to all 1s
  for (int i = 0; i < 11; i++) {
    int buttonState = digitalRead(BUTTON_PINS[i]);
    if (buttonState == LOW) { // Button pressed (since pull-up, pressed is LOW)
      newButtonState &= ~(1 << i); // Set bit to 0
    } else {
      newButtonState |= (1 << i);  // Set bit to 1
    }
  }

  // Check if button state has changed
  if (newButtonState != lastButtonState) {
    currentButtonState = newButtonState;
    // Send the new button state to the master console
    sendButtonState();
    // Print the current button state
    printButtonState(currentButtonState);
    lastButtonState = currentButtonState;
  }

  delay(10); // Small delay to debounce buttons
}

void sendButtonState() {
  // Prepare data packet
  uint16_t dataToSend = currentButtonState;

  // Send data via ESP-NOW
  esp_err_t result = esp_now_send(masterMACAddress, (uint8_t *) &dataToSend, sizeof(dataToSend));

  if (result == ESP_OK) {
    Serial.println("Button state sent successfully.");
  } else {
    Serial.println("Error sending the button state.");
  }
}

void printButtonState(uint16_t state) {
  // Print binary representation
  Serial.print("Current Button State (BIN): ");
  Serial.println(String(state, BIN));

  // Print human-readable button states
  Serial.print("Current Button State: ");
  bool first = true;
  for (int i = 0; i < 11; i++) {
    bool pressed = !(state & (1 << i)); // 0 means pressed
    if (pressed) {
      if (!first) {
        Serial.print(", ");
      }
      Serial.print(buttonNames[i]);
      first = false;
    }
  }
  if (first) {
    Serial.print("None");
  }
  Serial.println();
}
