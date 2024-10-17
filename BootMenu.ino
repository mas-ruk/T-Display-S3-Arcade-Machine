#include <TFT_eSPI.h>
#include "Tetris.h"
#include "Pong.h"
#include "Snake.h"
#include "Chess.h"
#include <SPI.h>
#include <SD.h>

const int screenWidth = 170;
const int screenHeight = 320;

// Initialize TFT Object
TFT_eSPI tft = TFT_eSPI();

// SD card input pins
int csPin = 10;
int mosiPin = 11;
int sckPin = 12;
int misoPin = 13;

// Pseudocontroller buttons
int leftButton = 43;
int upButton = 44;
int downButton = 18;
int rightButton = 17;
int aButton = 21;
int bButton = 16;

int pauseButton = 1;
bool paused = false;

int mButton = 50;
int pButton = 51;
int xButton = 52;
int yButton = 53;

// Menu variables
String games[] = {"Tetris", "Pong", "Snake", "Chess", "Leaderboard"};
int currSelect = 0;
int totalGames = sizeof(games) / sizeof(games[0]);

// Define screen states
enum Screen {
  MENU,
  LEADERBOARD,
  TETRIS,
  PONG,
  SNAKE,
  CHESS
};

Screen currentScreen = MENU; // Initialize to MENU

// Previous button states for debouncing
bool preUpState = HIGH;
bool preDownState = HIGH;
bool preAState = HIGH;

// =============================================================================================================

void drawMenu() {
  // Set text size to 2 for the menu
  tft.setTextSize(2);
  tft.setRotation(0);
  
  // Initialize screen as black
  tft.fillScreen(TFT_BLACK);
  
  for (int i = 0; i < totalGames; i++) {
    if (i == currSelect) {
      tft.setTextColor(TFT_RED); // Highlight selected game in red
      tft.drawString("->", 5, 20 + i * 20);
    } else {
      tft.setTextColor(TFT_WHITE); // Other games in white
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
    // Example condition to exit the game
    if (isPauseButtonPressed()) {
      break; // Exit the game loop
    }
  }
  currentScreen = MENU; // Return to menu after exiting the game
}

void launchPong() {
  tft.fillScreen(TFT_BLACK);
  pongSetup();
  while(true) {
    pongLoop();
    // Example condition to exit the game
    if (isPauseButtonPressed()) {
      break; // Exit the game loop
    }
  }
  currentScreen = MENU; // Return to menu after exiting the game
}

void launchSnake() {
  tft.fillScreen(TFT_BLACK);
  snakeSetup();
  while (true) {
    snakeLoop();
    // Example condition to exit the game
    if (isPauseButtonPressed()) {
      break; // Exit the game loop
    }
  }
  currentScreen = MENU; // Return to menu after exiting the game
}

void launchChess() {
  tft.fillScreen(TFT_BLACK);
  chessSetup();
  while (true) {
    if (isPauseButtonPressed()) {
      break; // Exit the game loop
    }
    chessLoop();
  }
  currentScreen = MENU; // Return to menu after exiting the game
}

// =============================================================================================================

// Will return true if pause button pressed
bool isPauseButtonPressed() {
  return digitalRead(pauseButton) == LOW;
}

// =============================================================================================================

struct ScoreEntry {
  String initials;
  int score;
};

// =============================================================================================================

void displayLeaderboard(const char* filename) {
  Serial.println("Displaying Leaderboard: " + String(filename));
  
  // Clear the screen
  tft.fillScreen(TFT_BLACK);
  
  // Set text size to smallest (1) for leaderboard
  tft.setTextSize(1);
  
  // Set title
  tft.setTextColor(TFT_YELLOW);
  String title = "Leaderboard";
  int titleWidth = tft.textWidth(title);
  int titleX = (screenWidth - titleWidth) / 2; // Center horizontally
  tft.drawString(title, titleX, 10);
  
  // Open the score file
  File file = SD.open(filename);
  if (!file) {
    tft.setTextColor(TFT_RED);
    String errorMsg = "No scores available.";
    int errorWidth = tft.textWidth(errorMsg);
    int errorX = (screenWidth - errorWidth) / 2; // Center horizontally
    tft.drawString(errorMsg, errorX, 30);
    delay(2000);
    return;
  }
  
  // Read all scores into an array
  const int maxEntries = 100; // Maximum number of entries to read
  ScoreEntry scoresArray[maxEntries];
  int count = 0;
  
  while (file.available() && count < maxEntries) {
    String line = file.readStringUntil('\n');
    line.trim(); // Remove any leading/trailing whitespace
    if (line.length() < 5) continue; // Minimum length check (e.g., "AAA 0")
    
    int spaceIndex = line.lastIndexOf(' ');
    if (spaceIndex > 0 && spaceIndex < line.length() - 1) {
      String initials = line.substring(0, spaceIndex);
      int score = line.substring(spaceIndex + 1).toInt();
      
      // Limit initials to 3 characters
      if (initials.length() > 3) {
        initials = initials.substring(0, 3);
      }
      
      // Limit score to 5 digits
      String scoreStr = String(score);
      if (scoreStr.length() > 5) {
        scoreStr = scoreStr.substring(0, 5);
        score = scoreStr.toInt();
      }
      
      scoresArray[count].initials = initials;
      scoresArray[count].score = score;
      count++;
    }
  }
  file.close();
  
  // Sort the scores in descending order using Bubble Sort (simple for small datasets)
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (scoresArray[j].score < scoresArray[j + 1].score) {
        // Swap
        ScoreEntry temp = scoresArray[j];
        scoresArray[j] = scoresArray[j + 1];
        scoresArray[j + 1] = temp;
      }
    }
  }
  
  // Display the top 10 scores
  tft.setTextColor(TFT_WHITE);
  int y = 30; // Starting y position for scores
  int displayLimit = (count < 10) ? count : 10;
  
  for (int i = 0; i < displayLimit; i++) {
    // Create a concise display line: "1. AAA 1000"
    String displayLine = String(i + 1) + ". " + scoresArray[i].initials + " " + String(scoresArray[i].score);
    
    // Calculate text width and adjust x position to center
    int lineWidth = tft.textWidth(displayLine);
    
    // If the line is still too wide, truncate it further
    if (lineWidth > screenWidth - 10) { // 10 pixels padding
      // Estimate maximum characters that fit within screenWidth - 10
      int maxChars = (screenWidth - 10) / 6; // Approximate based on text size 1 (6 pixels per character)
      if (displayLine.length() > maxChars) {
        displayLine = displayLine.substring(0, maxChars - 3) + "...";
        lineWidth = tft.textWidth(displayLine);
      }
    }
    
    int lineX = (screenWidth - lineWidth) / 2; // Center horizontally
    tft.drawString(displayLine, lineX, y);
    y += 15; // Increment y position for the next line (adjusted for smaller text size)
  }
  
  if (displayLimit == 0) {
    tft.setTextColor(TFT_RED);
    String noScoresMsg = "No scores available.";
    int noScoresWidth = tft.textWidth(noScoresMsg);
    int noScoresX = (screenWidth - noScoresWidth) / 2; // Center horizontally
    tft.drawString(noScoresMsg, noScoresX, 30);
  }
  
  // Prompt to return
  tft.setTextColor(TFT_WHITE);
  String prompt = "Press A to return";
  int promptWidth = tft.textWidth(prompt);
  int promptX = (screenWidth - promptWidth) / 2; // Center horizontally
  tft.drawString(prompt, promptX, 310); // Adjust y position if needed
}

// =============================================================================================================

void handleMenu() {
  // Draw the menu if needed (e.g., on state entry)
  static bool menuDrawn = false;
  if (!menuDrawn) {
    drawMenu();
    menuDrawn = true;
  }
  
  // Read button states
  bool currUpState = digitalRead(upButton);
  bool currDownState = digitalRead(downButton);
  bool currAState = digitalRead(aButton);

  // Navigate Up
  if (currUpState == LOW && preUpState == HIGH) {
    currSelect = (currSelect - 1 + totalGames) % totalGames; // Wrap around to last game
    drawMenu();
    menuDrawn = true;
    delay(200); // Debounce
  }

  // Navigate Down
  if (currDownState == LOW && preDownState == HIGH) {
    currSelect = (currSelect + 1) % totalGames; // Wrap around to first game
    drawMenu();
    menuDrawn = true;
    delay(200); // Debounce
  }

  // Select Option
  if (currAState == LOW && preAState == HIGH) {
    menuDrawn = false; // Reset for next time entering menu
    switch(currSelect) {
      case 0:
        currentScreen = TETRIS;
        break;
      case 1:
        currentScreen = PONG;
        break;
      case 2:
        currentScreen = SNAKE;
        break;
      case 3:
        currentScreen = CHESS;
        break;
      case 4:
        currentScreen = LEADERBOARD;
        break;
    }
    delay(200); // Debounce
  }

  // Update previous states
  preUpState = currUpState;
  preDownState = currDownState;
  preAState = currAState;

  // Debugging input
  Serial.print("Down state: ");
  Serial.println(currDownState);
  Serial.print("Up state: ");
  Serial.println(currUpState);
  Serial.print("A state: ");
  Serial.println(currAState);

  delay(50);
}

// =============================================================================================================

void handleLeaderboard() {
  static bool leaderboardDrawn = false;
  
  if (!leaderboardDrawn) {
    displayLeaderboard("leaderboard.txt"); // Replace with your actual leaderboard file name
    leaderboardDrawn = true;
  }

  // Read 'A' button state
  bool currAState = digitalRead(aButton);
  
  // Detect 'A' button press
  if (currAState == LOW && preAState == HIGH) {
    Serial.println("'A' button pressed on Leaderboard. Returning to Menu.");
    currentScreen = MENU; // Return to menu
    leaderboardDrawn = false; // Reset for next time
    delay(200); // Debounce
  }

  // Update previous 'A' button state
  preAState = currAState;

  delay(50);
}

// =============================================================================================================

void showLeaderboard() {
  currentScreen = LEADERBOARD;
}

// =============================================================================================================

void setup() {
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  Serial.begin(115200);
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(4); // Upside down for convenience, change this later
  tft.setTextSize(2);
  
  // Setup SD card
  if (!SD.begin(csPin)) {
    Serial.println("SD card initialization failed!");
    tft.setTextColor(TFT_RED);
    tft.drawString("SD Init Failed!", 10, 150);
    while (true); // Halt execution
  }
  Serial.println("SD card initialized.");
  
  // Set I/O pins
  pinMode(37, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(aButton, INPUT_PULLUP);
  pinMode(bButton, INPUT_PULLUP);
  pinMode(pauseButton, INPUT_PULLUP);
  
  // Random seed for food generation in snake
  randomSeed(analogRead(0));
  
  drawMenu();
}

// =============================================================================================================

void loop() {
  switch(currentScreen) {
    case MENU:
      handleMenu();
      break;
      
    case LEADERBOARD:
      handleLeaderboard();
      break;
      
    case TETRIS:
      launchTetris();
      break;
      
    case PONG:
      launchPong();
      break;
      
    case SNAKE:
      launchSnake();
      break;
      
    case CHESS:
      launchChess();
      break;
  }
}
