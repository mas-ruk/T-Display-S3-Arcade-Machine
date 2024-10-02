#include <TFT_eSPI.h>
#include <Arduino.h>
#include <esp_system.h>
#include "Pong.h"

extern TFT_eSPI tft;
extern bool paused;
extern bool isPauseButtonPressed();

// Global variables
Paddle player1, player2;
Ball ball;
int player1Score = 0, player2Score = 0;
int selectedOption = 0;

// =============================================================================================================

// debounce vars
unsigned long p1LastUpTime = 0;
unsigned long p1LastDownTime = 0;
unsigned long p2LastUpTime = 0;
unsigned long p2LastDownTime = 0;

// =============================================================================================================

void drawPauseMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE); // reset colour
  tft.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 80);
  tft.println("GAME PAUSED");

  if (selectedOption == 0) {
    tft.setTextColor(TFT_RED);
  }
  tft.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 40);
  tft.println("Resume Game?");

  tft.setTextColor(TFT_WHITE); // reset colour
  if (selectedOption == 1) {
    tft.setTextColor(TFT_RED);
  }
  tft.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
  tft.println("Restart Game?");

  tft.setTextColor(TFT_WHITE); // reset colour
  if (selectedOption == 2) {
    tft.setTextColor(TFT_RED);
  }
  tft.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + 40);
  tft.println("Return to menu?");

  tft.setTextColor(TFT_WHITE);
  
  // small delay to reduce flickering
  delay(20);
}

// =============================================================================================================

void handlePauseMenu() {
  // using up down buttons to navigate pause menu
  if (digitalRead(player1Up) == HIGH || digitalRead(player2Up) == HIGH) {
    selectedOption = (selectedOption + 1) % 3;
    delay(100); // debounce
  }
  if (digitalRead(player1Down) == HIGH || digitalRead(player2Down) == HIGH) {
    selectedOption = (selectedOption - 1 + 3) % 3;
    delay(100); // debounce
  }

  static bool lastAState = HIGH;
  bool currAState = digitalRead(aButton);
  if (lastAState == HIGH && currAState == LOW) {
    delay(100);
    // Check what is currently selected
        if (selectedOption == 0) { // Resume Game
            paused = false;
        } else if (selectedOption == 1) { // Restart Game
            pongSetup(); // Reset the game
            paused = false;
        } else if (selectedOption == 2) { // Return to Boot Menu
            esp_restart();
        }
  }
  lastAState = currAState;
}

// =============================================================================================================

void gameOver() {
  int scrWidth = tft.width();
  int scrHeight = tft.height();

  String gameOver = "GAME OVER";
  String winnerText = (player1Score == 10) ? "Player 1 Wins!!!" : "Player 2 Wins!!!";

  if (player1Score == 10 || player2Score == 10) {
    tft.fillScreen(TFT_BLACK);

    // calc centered pos of text
    int gameOverX = (scrWidth - tft.textWidth(gameOver)) / 2;
    int winTextX = (scrWidth - tft.textWidth(winnerText)) / 2;

    // game over text
    tft.drawString(gameOver, gameOverX, scrHeight / 2);

    // winners text 10 pixels below
    tft.drawString(winnerText, winTextX, (scrHeight / 2) + 20);
  }
}
// =============================================================================================================

void pongSetup() {
    pinMode(player2Up, INPUT_PULLUP);
    pinMode(player2Down, INPUT_PULLUP);
    
    player1Score = 0;
    player2Score = 0;

    player1.x = 10;
    player1.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;

    player2.x = SCREEN_WIDTH - 20;
    player2.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;

    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.dx = 4; // Ball speed in x direction
    ball.dy = 4; // Ball speed in y direction
}

// =============================================================================================================

void pongDraw() {
    // Clear only the paddle areas
    tft.fillRect(player1.x, player1.y, PADDLE_WIDTH, PADDLE_HEIGHT, TFT_BLACK); // Erase previous paddle position
    tft.fillRect(player2.x, player2.y, PADDLE_WIDTH, PADDLE_HEIGHT, TFT_BLACK); // Erase previous paddle position

    // Draw paddles in the new positions
    tft.fillRect(player1.x, player1.y, PADDLE_WIDTH, PADDLE_HEIGHT, TFT_WHITE);
    tft.fillRect(player2.x, player2.y, PADDLE_WIDTH, PADDLE_HEIGHT, TFT_WHITE);

    // Clear only the ball area
    tft.fillRect(ball.x, ball.y, BALL_SIZE, BALL_SIZE, TFT_BLACK); // Erase previous ball position
    
    // Draw ball in the new position
    tft.fillRect(ball.x, ball.y, BALL_SIZE, BALL_SIZE, TFT_WHITE);

    // Draw scores without clearing the entire screen
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(SCREEN_WIDTH / 4, 10);
    tft.print(player1Score);
    tft.setCursor(3 * SCREEN_WIDTH / 4, 10);
    tft.print(player2Score);
}

// =============================================================================================================

void resetBall() {
  ball.x = SCREEN_WIDTH / 2;
  ball.y = SCREEN_HEIGHT / 2;
  ball.dx = 4; // Ball speed in x direction
  ball.dy = 4; // Ball speed in y direction
}

// =============================================================================================================

void pongUpdate() {
    if (!paused) {
        // Update ball position
      ball.x += ball.dx;
      ball.y += ball.dy;

      // Ball collision with top and bottom
      if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - BALL_SIZE) {
          ball.dy *= -1;
      }

      // Ball collision with paddles
      if (ball.dx < 0) {  // Ball moving left
          if (ball.x <= player1.x + PADDLE_WIDTH && ball.y + BALL_SIZE >= player1.y && ball.y <= player1.y + PADDLE_HEIGHT) {
              ball.dx *= -1;
              ball.x = player1.x + PADDLE_WIDTH; // Ensure ball is outside the paddle after bounce
          } else if (ball.x < 0) {  // If ball gets behind player1
              player2Score++;
              resetBall(); // Reset after scoring
              return;
          }
      } else if (ball.dx > 0) {  // Ball moving right
          if (ball.x + BALL_SIZE >= player2.x && ball.y + BALL_SIZE >= player2.y && ball.y <= player2.y + PADDLE_HEIGHT) {
              ball.dx *= -1;
              ball.x = player2.x - BALL_SIZE; // Ensure ball is outside the paddle after bounce
          } else if (ball.x > SCREEN_WIDTH - BALL_SIZE) {  // If ball gets behind player2
              player1Score++;
              resetBall(); // Reset after scoring
              return;
          }
      }

      // Ball out of bounds
      if (ball.x < 0) {
          player2Score++;
          resetBall(); // Reset
      } else if (ball.x > SCREEN_WIDTH - BALL_SIZE) {
          player1Score++;
          resetBall(); // Reset
    }
  }
}

// =============================================================================================================

void pongLoop() {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastUpdateTime;

  if (isPauseButtonPressed()) {
    delay(200);
    paused = !paused;
  }

  if (paused) {
    drawPauseMenu();
    handlePauseMenu();
  } else {
    
    // game over check
    if (player1Score == 10 || player2Score == 10) {
      gameOver();
      return; // stops loop
    }

    // Read inputs for paddles
    if (digitalRead(player1Down) == HIGH) {
        if (player1.y > 0) {  // Check upper bound
            player1.y -= 6;   // Move up
        }
    }
    if (digitalRead(player1Up) == HIGH) {
        if (player1.y < SCREEN_HEIGHT - PADDLE_HEIGHT) { // Check lower bound
            player1.y += 6;   // Move down
        }
    }
    if (digitalRead(player2Down) == HIGH) {
        if (player2.y > 0) {  // Check upper bound
            player2.y -= 6;   // Move up
        }
    }
    if (digitalRead(player2Up) == HIGH) {
        if (player2.y < SCREEN_HEIGHT - PADDLE_HEIGHT) { // Check lower bound
            player2.y += 6;   // Move down
        }
    }

    pongUpdate();
    tft.fillScreen(TFT_BLACK);
    pongDraw();
  }
  delay(20);
}

// =============================================================================================================
