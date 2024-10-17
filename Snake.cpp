// Snake.cpp

#include "Snake.h"
#include <esp_system.h>
#include <SD.h>
#include <String.h>

// Screen dimensions for portrait mode
const int screenWidth = 170;   // Width of the TFT display in portrait
const int screenHeight = 320;  // Height of the TFT display in portrait

// Game grid dimensions
const int gridSize = 10;  // Size of each grid square in pixels

// Offset for score display
const int yOffset = 20; // Pixels reserved at the top for the score

const int gridWidth = screenWidth / gridSize;
const int gridHeight = (screenHeight - yOffset) / gridSize; // Adjusted for score offset

// Snake variables
int snakeX[100];  // Snake's x positions
int snakeY[100];  // Snake's y positions
int snakeLength;  // Length of the snake

// Food position
int foodX;
int foodY;

// Direction variables
int dirX;
int dirY;

// Game state
bool gameOver;

// Score variable
int snakeScore;

// Function prototypes (private to this file)
void readInputs();
void moveSnake();
void checkCollisions();
void drawGame();
void showGameOver();
void placeFood();

void snakeSetup() {
  // Initialize the game variables
  snakeLength = 3;
  
  // Initialize the score
  snakeScore = 0;

  // Start the snake in the middle of the screen
  snakeX[0] = gridWidth / 2;
  snakeY[0] = gridHeight / 2;

  // Initialize the direction (moving upwards)
  dirX = 0;
  dirY = -1;

  // Initialize the rest of the snake body behind the head
  for (int i = 1; i < snakeLength; i++) {
    snakeX[i] = snakeX[0];
    snakeY[i] = snakeY[0] + i;
  }

  // Place the food at a random position
  placeFood();

  // Set the screen rotation to portrait mode
  tft.setRotation(4);  // Adjust this value based on your display's orientation

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  gameOver = false;
}

void snakeLoop() {
  Serial.println("Snake Loop Running");
  while (!gameOver) {
    // Game logic
    readInputs();
    moveSnake();
    checkCollisions();
    drawGame();

    delay(100); // Control speed
  }

  if (gameOver) {
    snakeGameOver(snakeScore);
    return; // Exit the loop to return to the menu
  }
}

void readInputs() {
  // Read the direction buttons
  if (digitalRead(upButton) == LOW && dirY != 1) {
    dirX = 0;
    dirY = -1;
  } else if (digitalRead(downButton) == LOW && dirY != -1) {
    dirX = 0;
    dirY = 1;
  } else if (digitalRead(leftButton) == LOW && dirX != 1) {
    dirX = -1;
    dirY = 0;
  } else if (digitalRead(rightButton) == LOW && dirX != -1) {
    dirX = 1;
    dirY = 0;
  }
}

void moveSnake() {
  // Move the body
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // Move the head
  snakeX[0] += dirX;
  snakeY[0] += dirY;
}

void checkCollisions() {
  // Check collision with walls
  if (snakeX[0] < 0 || snakeX[0] >= gridWidth || snakeY[0] < 0 || snakeY[0] >= gridHeight) {
    gameOver = true;
    return;
  }

  // Check collision with self
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
      return;
    }
  }

  // Check if snake eats the food
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    // Add new segment at the end
    snakeX[snakeLength - 1] = snakeX[snakeLength - 2];
    snakeY[snakeLength - 1] = snakeY[snakeLength - 2];
    
    // Increment score
    snakeScore++;

    placeFood();
  }
}

void drawGame() {
  // Clear screen
  tft.fillScreen(TFT_BLACK);

  // Draw the score at the top center
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawCentreString("Score: " + String(snakeScore), screenWidth / 2, 5, 1); // GFXFF is the font

  // Draw snake
  for (int i = 0; i < snakeLength; i++) {
    int x = snakeX[i] * gridSize;
    int y = snakeY[i] * gridSize + yOffset; // Apply yOffset for score space
    tft.fillRect(x, y, gridSize, gridSize, TFT_GREEN);
  }

  // Draw food
  int foodPosX = foodX * gridSize;
  int foodPosY = foodY * gridSize + yOffset; // Apply yOffset for score space
  tft.fillRect(foodPosX, foodPosY, gridSize, gridSize, TFT_RED);
}

void placeFood() {
  foodX = random(0, gridWidth);
  foodY = random(0, gridHeight);
}

// ============================================== GAME OVER AND LEADERBOARD FUNCTIONALITY ==============================================

void snakeGameOver(int score) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(1);
  tft.drawString("Game Over!", 10, 10);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Score: " + String(score), 10, 40);
  tft.drawString("Save score? (A:Yes B:No)", 10, 70);

  while (true) {
    if (digitalRead(aButton) == LOW) {
      String initials = getInitials();
      saveScore("snake_scores.txt", initials, score);
      break;
    } else if (digitalRead(bButton) == LOW) {
      esp_restart();
    }
    delay(100);
  }
}

String getInitials() {
  char initials[4] = {'A', 'A', 'A', '\0'};
  int pos = 0;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Enter Initials:", 10, 10);

  while (true) {
    // Display initials
    tft.fillRect(10, 40, 100, 30, TFT_BLACK);
    tft.drawString(String(initials), 10, 40);

    // Read inputs
    if (digitalRead(upButton) == LOW) {
      initials[pos]++;
      if (initials[pos] > 'Z') initials[pos] = '0';
      if (initials[pos] > '9' && initials[pos] < 'A') initials[pos] = 'A';
      delay(200); // Debounce
    } else if (digitalRead(downButton) == LOW) {
      initials[pos]--;
      if (initials[pos] < '0') initials[pos] = 'Z';
      if (initials[pos] < 'A' && initials[pos] > '9') initials[pos] = '9';
      delay(200); // Debounce
    } else if (digitalRead(rightButton) == LOW) {
      pos = (pos + 1) % 3;
      delay(200); // Debounce
    } else if (digitalRead(leftButton) == LOW) {
      pos = (pos - 1 + 3) % 3;
      delay(200); // Debounce
    } else if (digitalRead(aButton) == LOW) {
      break; // Finish input
    }
    delay(50);
  }

  return String(initials);
}

void saveScore(const char* filename, String initials, int score) {
  File file = SD.open(filename, FILE_APPEND);
  if (file) {
    file.println(initials + " " + String(score));
    file.close();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.drawString("Score Saved!", 10, 150);
    delay(1000);
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.drawString("Save Failed!", 10, 150);
    delay(1000);
  }
}


