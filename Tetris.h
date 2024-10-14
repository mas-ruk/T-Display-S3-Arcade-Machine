#ifndef TETRIS_H
#define TETRIS_H

#include <SPI.h>
#include <TFT_eSPI.h>
#include "Buttons.h"
// Function declarations
void setup();
void loop();
void make_block(int n, uint16_t color);
void PutStartPos();
void Draw();
void GameOver();
void DeleteLine();

struct Point {int X, Y;};
struct Block {Point square[4][4]; int numRotate, color;};

// External declarations for global variables
extern TFT_eSPI tft; // Declare TFT object

void ReviseScreen(Point next_pos, int next_rot);
bool GetSquares(Block block, Point pos, int rot, Point* squares);

#endif // TETRIS_H
