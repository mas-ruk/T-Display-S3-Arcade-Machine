// ControllerInput.h

#ifndef CONTROLLER_INPUT_H
#define CONTROLLER_INPUT_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Maximum number of controllers
#define MAX_CONTROLLERS 2

// Controller 1 button states
extern int leftButton;
extern int rightButton;
extern int upButton;
extern int downButton;
extern int xButton;
extern int yButton;
extern int aButton;
extern int bButton;
extern int mButton;
extern int pButton;
extern int pauseButton;

// Controller 2 button states
extern int leftButton2;
extern int rightButton2;
extern int upButton2;
extern int downButton2;
extern int xButton2;
extern int yButton2;
extern int aButton2;
extern int bButton2;
extern int mButton2;
extern int pButton2;
extern int pauseButton2;
// Structure to hold controller data
/*
typedef struct {
  uint8_t macAddress[6];
  uint16_t buttonState; // Current button state
  bool connected;       // Connection status
} Controller;
*/
// Extern declarations
//extern Controller controllers[MAX_CONTROLLERS];

// Function to initialize controller input
void initControllerInput();

// Function to process controller input (call in loop if needed)
void updateControllerInput();

#endif // CONTROLLER_INPUT_H
