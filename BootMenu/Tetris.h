// Tetris.h

#ifndef TETRIS_H
#define TETRIS_H

#include <SPI.h>
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
extern ScoreEntry tetrisScores[5];

// Structure definitions
struct Point {
    int X, Y;
};

struct Block {
    Point square[4][4];
    int numRotate;
    int color;
};

// Function prototypes
void tetrisSetup();
void tetrisLoop();

// Function prototypes for functions used in tetris.ino
void GetNextPosRot(Point* pnext_pos, int* pnext_rot);
void ReviseScreen(Point next_pos, int next_rot);
bool GetSquares(Block block, Point pos, int rot, Point* squares);
void DeleteLine();
void PutStartPos();
void Draw();
void KeyPadLoop();
void GameOver();
void make_block(int n, uint16_t color);

#endif // TETRIS_H
