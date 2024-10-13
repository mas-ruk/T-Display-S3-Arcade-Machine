// Snake.h

#ifndef SNAKE_H
#define SNAKE_H

#include <TFT_eSPI.h>

// Externally declare the TFT display object from bootmenu.ino
extern TFT_eSPI tft;

// Externally declare the button pins from bootmenu.ino
extern int leftButton;
extern int rightButton;
extern int upButton;
extern int downButton;
extern int aButton;
extern int bButton;

// Function prototypes
void snakeSetup();
void snakeLoop();

#endif // SNAKE_H
