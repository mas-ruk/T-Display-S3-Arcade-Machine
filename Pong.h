#ifndef PONG_H
#define PONG_H

// screen size
const int SCREEN_WIDTH = 320; 
const int SCREEN_HEIGHT = 170;

// paddle size
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 60;

// ball size
const int BALL_SIZE = 8;

// Define button pins
const int player1Up = 16; // same as bootmenu
const int player1Down = 12; // same as bootmenu
const int player2Up = 1;
const int player2Down = 10;
extern int aButton;
extern int pauseButton;

// Declare variables
struct Paddle {
    int x, y;
};

struct Ball {
    int x, y, dx, dy;
};

void pongSetup();
void pongLoop();
extern void launchPong();
extern TFT_eSPI tft;
extern bool paused;
extern bool isPauseButtonPressed();

#endif
