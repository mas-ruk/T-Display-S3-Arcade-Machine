// Snake.cpp

#include "Snake.h"
#include <esp_system.h>

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

void showGameOver() {
  // Clear the screen with black color
  tft.fillScreen(TFT_BLACK);
  
  // Set text color to white
  tft.setTextColor(TFT_WHITE);
  
  // Set text size for "Game Over"
  tft.setTextSize(2);
  
  // Draw "Game Over" at the center, slightly above the middle
  tft.drawCentreString("Game Over", screenWidth / 2, screenHeight / 2 - 50, 1);
  
  // Display the final score
  tft.setTextSize(2);
  
  // Set smaller text size for instructions
  tft.setTextSize(1);
  tft.drawCentreString("Final Score: " + String(snakeScore), screenWidth / 2, screenHeight / 2 - 20, 1);

  // Draw "Press A to Restart" below "Game Over"
  tft.drawCentreString("Press A to Restart", screenWidth / 2, screenHeight / 2 + 10, 1);
  
  // Draw "Press B for Menu" further below
  tft.drawCentreString("Press B for Menu", screenWidth / 2, screenHeight / 2 + 30, 1);
}

void placeFood() {
  foodX = random(0, gridWidth);
  foodY = random(0, gridHeight);
}
