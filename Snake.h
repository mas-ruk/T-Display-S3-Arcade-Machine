// Snake.h

#ifndef SNAKE_H
#define SNAKE_H

#include <TFT_eSPI.h>
#include "Buttons.h"
// Externally declare the TFT display object from bootmenu.ino
extern TFT_eSPI tft;


// Function prototypes
void snakeSetup();
void snakeLoop();

#endif // SNAKE_H
