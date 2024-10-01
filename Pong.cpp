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

void pongSetup() {
    player1.x = 10;
    player1.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;

    player2.x = SCREEN_WIDTH - 20;
    player2.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;

    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.dx = 2; // Ball speed in x direction
    ball.dy = 2; // Ball speed in y direction
}

// =============================================================================================================

void pongDraw() {
    // Draw paddles
    tft.fillRect(player1.x, player1.y, PADDLE_WIDTH, PADDLE_HEIGHT, TFT_WHITE);
    tft.fillRect(player2.x, player2.y, PADDLE_WIDTH, PADDLE_HEIGHT, TFT_WHITE);

    // Draw ball
    tft.fillRect(ball.x, ball.y, BALL_SIZE, BALL_SIZE, TFT_WHITE);

    // Draw scores
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

  // Read inputs for paddles
  if (digitalRead(player1Down) == HIGH) {
      if (player1.y > 0) {  // Check upper bound
          player1.y -= 5;   // Move up
      }
  }
  if (digitalRead(player1Up) == HIGH) {
      if (player1.y < SCREEN_HEIGHT - PADDLE_HEIGHT) { // Check lower bound
          player1.y += 5;   // Move down
      }
  }
  if (digitalRead(player2Down) == HIGH) {
      if (player2.y > 0) {  // Check upper bound
          player2.y -= 5;   // Move up
      }
  }
  if (digitalRead(player2Up) == HIGH) {
      if (player2.y < SCREEN_HEIGHT - PADDLE_HEIGHT) { // Check lower bound
          player2.y += 5;   // Move down
      }
  }

  pongUpdate();
  tft.fillScreen(TFT_BLACK);
  pongDraw();
  delay(20);
}

// =============================================================================================================
