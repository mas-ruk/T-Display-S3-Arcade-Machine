#include <TFT_eSPI.h>
#include "Tetris.h"
#include "Pong.h"

// init tft Object
TFT_eSPI tft = TFT_eSPI();

// input pins
int leftButton = 43;
int rightButton = 44;
int upButton = 18;
int downButton = 17;

int aButton = 21;
int bButton = 16;

// menu vars
String games[] = {"Tetris", "Pong", "Game 3"};
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

void launchGame3() {
  
}

// =============================================================================================================

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1); // upside down for convience, change this later
  tft.setTextSize(2);

  // set i/o pins
  pinMode(37,INPUT_PULLUP);
  pinMode(leftButton,INPUT_PULLUP);
  pinMode(rightButton,INPUT_PULLUP);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(aButton, INPUT_PULLUP);

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
      launchGame3();
    }
    // add more games here
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
