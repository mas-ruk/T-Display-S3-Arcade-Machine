// Snake.h

#ifndef SNAKE_H
#define SNAKE_H

#include <TFT_eSPI.h>
#include <String.h>

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
void snakeGameOver(int score);
String getInitials();
void saveScore(const char* filename, String initials, int score);

extern void drawMenu();

#endif // SNAKE_H
