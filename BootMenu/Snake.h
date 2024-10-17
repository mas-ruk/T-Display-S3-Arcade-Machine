// Snake.h

#ifndef SNAKE_H
#define SNAKE_H

#include <TFT_eSPI.h>
#include "Scores.h" // Include Scores.h

// Externally declare the TFT display object
extern TFT_eSPI tft;

// Externally declare the button pins
extern int leftButton;
extern int rightButton;
extern int upButton;
extern int downButton;
extern int aButton;
extern int bButton;

// Externally declare score structures and functions
extern void insertNewScore(ScoreEntry scores[], int newScore);
extern void writeScoresToSD(const char* filename, ScoreEntry scores[]);
extern ScoreEntry snakeScores[5];

// Function prototypes
void snakeSetup();
void snakeLoop();

#endif // SNAKE_H
