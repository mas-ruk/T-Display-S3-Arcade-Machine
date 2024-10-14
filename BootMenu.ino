#include <TFT_eSPI.h>
#include "Tetris.h"
#include "Pong.h"
#include "Snake.h"

// init tft Object
TFT_eSPI tft = TFT_eSPI();

// input pins
int leftButton = 13;
int rightButton = 11;
int upButton = 16;
int downButton = 12;

int xButton = 1;
int yButton = 3;

int aButton = 2;
int bButton = 10;

int mButton = 44;
int pButton = 43;

int pauseButton = 21;

// pause
bool paused = false;

// menu vars
String games[] = {"Tetris", "Pong", "Snake"};
int currSelect = 0;
int totalGames = sizeof(games) / sizeof(games[0]);

// =============================================================================================================

void drawMenu() {
  // init screen as black
  tft.fillScreen(TFT_BLACK);
  for (int i = 0; i < totalGames; i++) {
    if (i == currSelect) {
      tft.setTextColor(TFT_RED); // if currently selected, highlight as red
      tft.drawString("->", 5, 20 + i * 20);
    } else {
      tft.setTextColor(TFT_WHITE); // otherwise white
    } 
    tft.drawString(games[i], 35, 20 + i * 20);
  }
}

// =============================================================================================================

void launchTetris() {
  tft.fillScreen(TFT_BLACK);
  tetrisSetup();
  while(true) {
    tetrisLoop();
  }
}

void launchPong() {
  tft.fillScreen(TFT_BLACK);
  pongSetup();
  while(true) {
    pongLoop();
  }
}

void launchSnake() {
  tft.fillScreen(TFT_BLACK);
  snakeSetup();
  while (true) {
    snakeLoop();
  }
}

// =============================================================================================================

// will return true if pause button pressed
bool isPauseButtonPressed() {
  return digitalRead(pauseButton) == LOW;
}

// =============================================================================================================

void setup() {
  pinMode(15, OUTPUT);
   digitalWrite(15, HIGH);
  Serial.begin(115200);
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(4); // upside down for convience, change this later
  tft.setTextSize(2);

  // set i/o pins
  pinMode(37,INPUT_PULLUP);
  pinMode(leftButton,INPUT_PULLUP);
  pinMode(rightButton,INPUT_PULLUP);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(aButton, INPUT_PULLUP);
  pinMode(bButton, INPUT_PULLUP);
  pinMode(pauseButton, INPUT_PULLUP);

  // random seed for food generation in snake
  randomSeed(analogRead(0));

  drawMenu();
}

// =============================================================================================================

bool preUpState = HIGH;
bool preDownState = HIGH;
bool preAState = HIGH;

void loop() {
  // init bools to states of buttons
  bool currUpState = digitalRead(upButton);
  bool currDownState = digitalRead(downButton);
  bool currAState = digitalRead(aButton);

  // up 
  if (currUpState == LOW && preUpState == HIGH) {
    currSelect = (currSelect - 1 + totalGames) % totalGames; // Wrap around to last game
    drawMenu();
  }

  // down
  if (currDownState == LOW && preDownState == HIGH) {
    currSelect = (currSelect + 1) % totalGames; // Wrap around to first game
    drawMenu();
  }

  // A button
  if (currAState == LOW && preAState == HIGH) {
    if (currSelect == 0) {
      launchTetris();
    } else if (currSelect == 1){
      launchPong();
    } else if (currSelect == 2){
      launchSnake();
    }
  }

  // update states 
  preUpState = currUpState;
  preDownState = currDownState;
  preAState = currAState;
  
  // debugging input
  Serial.print("Down state: ");
  Serial.println(currDownState);
  Serial.print("Up state: ");
  Serial.println(currUpState);
  Serial.print("A state: ");
  Serial.println(currAState);

  delay(50);
}
