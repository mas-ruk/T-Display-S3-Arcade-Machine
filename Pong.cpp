#include <TFT_eSPI.h>
#include <Arduino.h>
#include "Pong.h"

extern TFT_eSPI tft;

// Global variables
Paddle player1, player2;
Ball ball;
int player1Score = 0, player2Score = 0;

// =============================================================================================================

// debounce vars
unsigned long p1LastUpTime = 0;
unsigned long p1LastDownTime = 0;
unsigned long p2LastUpTime = 0;
unsigned long p2LastDownTime = 0;

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

void pongUpdate() {
    // Update ball position
    ball.x += ball.dx;
    ball.y += ball.dy;

    // Ball collision with top and bottom
    if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - BALL_SIZE) {
        ball.dy *= -1;
    }

    // Ball collision with paddles
    if ((ball.x <= player1.x + PADDLE_WIDTH && ball.y + BALL_SIZE >= player1.y && ball.y <= player1.y + PADDLE_HEIGHT) ||
        (ball.x + BALL_SIZE >= player2.x && ball.y + BALL_SIZE >= player2.y && ball.y <= player2.y + PADDLE_HEIGHT)) {
        ball.dx *= -1;
    }

    // Ball out of bounds
    if (ball.x < 0) {
        player2Score++;
        pongSetup(); // Reset
    } else if (ball.x > SCREEN_WIDTH - BALL_SIZE) {
        player1Score++;
        pongSetup(); // Reset
    }
}

// =============================================================================================================

void pongLoop() {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastUpdateTime;

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
  delay(20);
}

// =============================================================================================================
