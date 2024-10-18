#include <WiFi.h>

void setup() {
  pinMode(15,OUTPUT);
  digitalWrite(15, HIGH);
  Serial.begin(115200);
  delay(2000);
  WiFi.mode(WIFI_STA);
  Serial.print("Master Console MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
    
}
