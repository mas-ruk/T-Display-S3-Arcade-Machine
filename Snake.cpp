// Snake.cpp

#include "Snake.h"
#include <esp_system.h>

// Screen dimensions (adjust based on your display)
const int screenWidth = 320;   // Width of the TFT display
const int screenHeight = 170;  // Height of the TFT display

// Game grid dimensions
const int gridSize = 10;  // Size of each grid square in pixels

const int gridWidth = screenWidth / gridSize;
const int gridHeight = screenHeight / gridSize;

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
  // Start the snake in the middle of the screen
  snakeX[0] = gridWidth / 2;
  snakeY[0] = gridHeight / 2;

  // Initialize the direction (moving to the right)
  dirX = 1;
  dirY = 0;

  // Initialize the rest of the snake body behind the head
  for (int i = 1; i < snakeLength; i++) {
    snakeX[i] = snakeX[0] - i;
    snakeY[i] = snakeY[0];
  }

  // Place the food at a random position
  placeFood();

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  gameOver = false;
}

void snakeLoop() {
  Serial.println("Snake Loop Running");
  while (!gameOver) {
    // Check if 'B' button is pressed to exit
    if (digitalRead(bButton) == LOW) {
      esp_restart();  // Restart the microcontroller
    }

    // Read inputs
    readInputs();
    moveSnake();
    checkCollisions();
    drawGame();

    // Delay to control game speed
    delay(100);  // Adjust speed as necessary
  }

  if (gameOver) {
    showGameOver();
    while (true) {
      // Wait for 'A' button to restart or 'B' button to exit to menu
      if (digitalRead(aButton) == LOW) {
        snakeSetup();
        return;
      } else if (digitalRead(bButton) == LOW) {
        esp_restart();  // Restart the microcontroller
      }
      delay(100);
    }
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
    placeFood();
  }
}

void drawGame() {
  // Clear screen
  tft.fillScreen(TFT_BLACK);

  // Draw snake
  for (int i = 0; i < snakeLength; i++) {
    int x = snakeX[i] * gridSize;
    int y = snakeY[i] * gridSize;
    tft.fillRect(x, y, gridSize, gridSize, TFT_GREEN);
  }

  // Draw food
  int foodPosX = foodX * gridSize;
  int foodPosY = foodY * gridSize;
  tft.fillRect(foodPosX, foodPosY, gridSize, gridSize, TFT_RED);
}

void showGameOver() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.drawString("Game Over", screenWidth / 2 - 60, screenHeight / 2 - 30);
  tft.setTextSize(2);
  tft.drawString("Press A to Restart", screenWidth / 2 - 70, screenHeight / 2 + 10);
  tft.drawString("Press B for Menu", screenWidth / 2 - 70, screenHeight / 2 + 40);
}

void placeFood() {
  foodX = random(0, gridWidth);
  foodY = random(0, gridHeight);
}
