// bootmenu.ino

#include <TFT_eSPI.h>
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>
#include "ControllerInput.h"
#include "Tetris.h"
#include "Pong.h"
#include "Snake.h"
#include "Chess.h"
#include "Scores.h" // Include Scores.h

// Initialize TFT object
TFT_eSPI tft = TFT_eSPI();

// Arrays to hold the top 5 scores for each game
ScoreEntry snakeScores[5];
ScoreEntry tetrisScores[5];

// Function prototypes for score handling
void insertNewScore(ScoreEntry scores[], int newScore);
void writeScoresToSD(const char* filename, ScoreEntry scores[]);
void readScoresFromSD(const char* filename, ScoreEntry scores[]);

// Menu variables
String games[] = {"Tetris", "Pong", "Snake", "Chess", "Scoreboard"};
int currSelect = 0;
int totalGames = sizeof(games) / sizeof(games[0]);

extern int pauseButton;
bool paused = false;

// =============================================================================================================

void drawMenu() {
  tft.setTextSize(2);
  // Initialize screen as black
  tft.fillScreen(TFT_BLACK);
  for (int i = 0; i < totalGames; i++) {
    if (i == currSelect) {
      tft.setTextColor(TFT_RED); // If currently selected, highlight as red
      tft.drawString("->", 5, 20 + i * 20);
    } else {
      tft.setTextColor(TFT_WHITE); // Otherwise white
    }
    tft.drawString(games[i], 35, 20 + i * 20);
  }
}

// =============================================================================================================

void launchTetris() {
  tft.fillScreen(TFT_BLACK);
  tetrisSetup();
  while (true) {
    if (isPauseButtonPressed()) {
      esp_restart(); // Return to menu when pause button is pressed
    }
    tetrisLoop();
  }
}

void launchPong() {
  tft.fillScreen(TFT_BLACK);
  pongSetup();
  while (true) {
    pongLoop();
  }
}

void launchSnake() {
  tft.fillScreen(TFT_BLACK);
  snakeSetup();
  while (true) {
    if (isPauseButtonPressed()) {
      esp_restart(); // Return to menu when pause button is pressed
    }
    snakeLoop();
  }
}

void launchChess() {
  tft.fillScreen(TFT_BLACK);
  chessSetup();
  while (true) {
    if (isPauseButtonPressed()) {
      esp_restart(); // Return to menu when pause button is pressed
    }
    chessLoop();
  }
  drawMenu();
}

// =============================================================================================================

// Will return true if pause button pressed
int isPauseButtonPressed() {
  return pauseButton == 0;
}

// =============================================================================================================

void setup() {
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  Serial.begin(115200);
  delay(1000);
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(4); // Adjust rotation as needed
  tft.setTextSize(2);
  initControllerInput();

  // Random seed for food generation in snake
  randomSeed(analogRead(0));

  // Initialize SD card
  if (!SD.begin(/* CS pin number */)) {
    Serial.println("Card Mount Failed");
    // Handle SD card initialization failure
  } else {
    Serial.println("SD Card initialized.");

    // Read high scores from SD card
    readScoresFromSD("/snake_scores.txt", snakeScores);
    readScoresFromSD("/tetris_scores.txt", tetrisScores);
  }

  drawMenu();
}

// =============================================================================================================

// Function to read scores from SD card
void readScoresFromSD(const char* filename, ScoreEntry scores[]) {
  File file = SD.open(filename, FILE_READ);
  if (file) {
    int index = 0;
    while (file.available() && index < 5) {
      String line = file.readStringUntil('\n');
      line.trim();
      int separatorIndex = line.indexOf(',');
      if (separatorIndex != -1) {
        scores[index].name = line.substring(0, separatorIndex);
        scores[index].score = line.substring(separatorIndex + 1).toInt();
        index++;
      }
    }
    file.close();

    // If fewer than 5 scores were read, initialize the rest
    for (; index < 5; index++) {
      scores[index].name = "---";
      scores[index].score = 0;
    }
  } else {
    // If file doesn't exist, initialize with default values
    for (int i = 0; i < 5; i++) {
      scores[i].name = "---";
      scores[i].score = 0;
    }
  }
}

// Function to write scores to SD card
void writeScoresToSD(const char* filename, ScoreEntry scores[]) {
  SD.remove(filename); // Delete the file if it exists
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    for (int i = 0; i < 5; i++) {
      file.println(scores[i].name + "," + String(scores[i].score));
    }
    file.close();
  } else {
    Serial.println("Failed to open file for writing");
  }
}

// Function to insert a new score into the top 5 scores list
void insertNewScore(ScoreEntry scores[], int newScore) {
  // Prompt for player name or initials (simplified here)
  String playerName = getNameInput(); // Modify this to get input from the user if possible

  // Create a new score entry
  ScoreEntry newEntry;
  newEntry.name = playerName;
  newEntry.score = newScore;

  // Insert the new entry into the array
  for (int i = 0; i < 5; i++) {
    if (newScore > scores[i].score) {
      // Shift lower scores down
      for (int j = 4; j > i; j--) {
        scores[j] = scores[j - 1];
      }
      scores[i] = newEntry;
      break;
    }
  }
}

// =============================================================================================================

int preUpState = 1;
int preDownState = 1;
int preAState = 1;

void loop() {
  updateControllerInput();
  // Init bools to states of buttons
  int currUpState = upButton;
  int currDownState = downButton;
  int currAState = aButton;

  // Up
  if (currUpState == 0 && preUpState == 1) {
    currSelect = (currSelect - 1 + totalGames) % totalGames; // Wrap around to last game
    drawMenu();
  }

  // Down
  if (currDownState == 0 && preDownState == 1) {
    currSelect = (currSelect + 1) % totalGames; // Wrap around to first game
    drawMenu();
  }

  // A button
  if (currAState == 0 && preAState == 1) {
    if (currSelect == 0) {
      launchTetris();
    } else if (currSelect == 1) {
      launchPong();
    } else if (currSelect == 2) {
      launchSnake();
    } else if (currSelect == 3) {
      launchChess();
    } else if (currSelect == 4) {
      showScoreboard();
    }
  }

  // Update states
  preUpState = currUpState;
  preDownState = currDownState;
  preAState = currAState;
}

// =============================================================================================================

String getNameInput() {
  String playerName = "";
  const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  int charIndex = 0;
  int nameLength = 0;
  int maxNameLength = 3; // Maximum number of characters in the name
  bool selecting = true;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);

  while (selecting) {
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Enter Your Name", 10, 10);

    // Display the current name
    tft.drawString("Name: " + playerName, 10, 50);

    // Display the current character
    tft.drawString(String(characters[charIndex]), 60, 100);

    tft.drawString("Use LEFT/RIGHT to change", 10, 150);
    tft.drawString("Press A to select", 10, 170);
    tft.drawString("Press B to finish", 10, 190);

    // Wait for button input
    while (true) {
      updateControllerInput();

      if (leftButton == 0) {
        // Move to previous character
        charIndex = (charIndex - 1 + sizeof(characters) - 1) % (sizeof(characters) - 1);
        break;
      }

      if (rightButton == 0) {
        // Move to next character
        charIndex = (charIndex + 1) % (sizeof(characters) - 1);
        break;
      }

      if (aButton == 0 && nameLength < maxNameLength) {
        // Add the selected character to the name
        playerName += characters[charIndex];
        nameLength++;
        break;
      }

      if (bButton == 0) {
        // Finish name input
        selecting = false;
        break;
      }

      delay(100); // Small delay to debounce buttons
    }

    delay(200); // Delay to prevent multiple inputs from a single press
  }

  // If the player didn't enter any name, set a default
  if (playerName.length() == 0) {
    playerName = "AAA";
  }

  return playerName;
}

// ==================================================================

// Function to display the scoreboard
void showScoreboard() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  // Display High Scores
  tft.drawString("High Scores", 20, 10);

  tft.setTextSize(1);
  int yPosition = 40;

  // Display Snake Scores
  tft.drawString("Snake:", 20, yPosition);
  yPosition += 15;
  for (int i = 0; i < 5; i++) {
    tft.drawString(String(i + 1) + ". " + snakeScores[i].name + " - " + String(snakeScores[i].score), 20, yPosition);
    yPosition += 15;
  }

  yPosition += 10;

  // Display Tetris Scores
  tft.drawString("Tetris:", 20, yPosition);
  yPosition += 15;
  for (int i = 0; i < 5; i++) {
    tft.drawString(String(i + 1) + ". " + tetrisScores[i].name + " - " + String(tetrisScores[i].score), 20, yPosition);
    yPosition += 15;
  }

  tft.drawString("Press B to return", 20, 240);

  // Wait for user to press 'B' to return to the menu
  while (true) {
    updateControllerInput();
    if (bButton == 0) {
      drawMenu();
      break;
    }
    delay(100);
  }
}

// =============================================================================================================
