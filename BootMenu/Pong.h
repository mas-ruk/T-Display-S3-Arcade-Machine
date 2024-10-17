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
extern int upButton; // same as bootmenu
extern int downButton; // same as bootmenu
extern int upButton2;
extern int downButton2;
extern int aButton;
extern int aButton2;
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

#endif
