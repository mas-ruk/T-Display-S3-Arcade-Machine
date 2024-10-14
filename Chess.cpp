#include "Chess.h"

// Definition of global variables
Piece board[8][8];
PlayerColor currentPlayer = WHITE;
int cursorX = 0;
int cursorY = 0;
int selectedX = -1;
int selectedY = -1;
bool prevLeftState = HIGH;
bool prevRightState = HIGH;
bool prevUpState = HIGH;
bool prevDownState = HIGH;
bool prevAState = HIGH;
bool prevBState = HIGH;

void chessSetup() {
  // Initialize the board with starting positions
  // Set up pieces for both players

  // Initialize all squares to empty
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      board[y][x].type = EMPTY;
      board[y][x].color = NONE;
    }
  }

  // Place pawns
  for (int x = 0; x < 8; x++) {
    board[1][x].type = PAWN;
    board[1][x].color = BLACK;
    board[6][x].type = PAWN;
    board[6][x].color = WHITE;
  }

  // Place other pieces
  PieceType backRank[] = { ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK };

  for (int x = 0; x < 8; x++) {
    // Black back rank
    board[0][x].type = backRank[x];
    board[0][x].color = BLACK;

    // White back rank
    board[7][x].type = backRank[x];
    board[7][x].color = WHITE;
  }

  // Draw the initial board
  drawBoard();
}

void drawBoard() {
  // Calculate square size based on screen dimensions
  int screenWidth = tft.width();   // Should be 240
  int screenHeight = tft.height(); // Should be 135

  int maxSquareSizeX = screenWidth / 8;  // 240 / 8 = 30
  int maxSquareSizeY = screenHeight / 8; // 135 / 8 ≈ 16

  int squareSize = min(maxSquareSizeX, maxSquareSizeY); // squareSize = 16

  int boardWidth = squareSize * 8;
  int boardHeight = squareSize * 8;

  int offsetX = (screenWidth - boardWidth) / 2;
  int offsetY = (screenHeight - boardHeight) / 2;

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      int posX = offsetX + x * squareSize;
      int posY = offsetY + y * squareSize;

      // Alternate colors for squares
      uint16_t squareColor = ((x + y) % 2 == 0) ? TFT_WHITE : TFT_LIGHTGREY;

      // Highlight selected square
      if (x == selectedX && y == selectedY) {
        squareColor = TFT_YELLOW;
      }

      // Highlight cursor position
      if (x == cursorX && y == cursorY) {
        squareColor = TFT_GREEN;
      }

      tft.fillRect(posX, posY, squareSize, squareSize, squareColor);

      // Draw piece if present
      if (board[y][x].type != EMPTY) {
        // Set piece color
        uint16_t pieceColor = (board[y][x].color == WHITE) ? TFT_WHITE : TFT_BLACK;

        // Draw the piece
        drawPiece(board[y][x], posX, posY, squareSize, pieceColor, squareColor);
      }

      // Highlight available moves if a piece is selected
      if (selectedX != -1 && selectedY != -1 && isLegalMove(selectedX, selectedY, x, y)) {
        tft.drawRect(posX, posY, squareSize, squareSize, TFT_BLUE);
      }
    }
  }
}

void drawPiece(Piece piece, int posX, int posY, int squareSize, uint16_t pieceColor, uint16_t bgColor) {
  int centerX = posX + squareSize / 2;
  int centerY = posY + squareSize / 2;
  int radius = squareSize / 3;
  
  // Determine the outline color
  uint16_t outlineColor = (piece.color == WHITE) ? TFT_BLACK : pieceColor;

  // Clear the square before drawing the piece
  tft.fillRect(posX, posY, squareSize, squareSize, bgColor);

  switch (piece.type) {
    case PAWN:
      // Draw outline circle
      tft.fillCircle(centerX, centerY, radius, outlineColor);
      // Draw inner circle slightly smaller
      tft.fillCircle(centerX, centerY, radius - 1, pieceColor);
      break;

    case KNIGHT:
      // Draw outline triangle
      tft.fillTriangle(
        centerX, centerY - radius,
        centerX - radius, centerY + radius,
        centerX + radius, centerY + radius,
        outlineColor
      );
      // Draw inner triangle slightly smaller
      tft.fillTriangle(
        centerX, centerY - radius + 1,
        centerX - radius + 1, centerY + radius - 1,
        centerX + radius - 1, centerY + radius - 1,
        pieceColor
      );
      break;

    case BISHOP:
      // Draw outline triangle
      tft.fillTriangle(
        centerX, centerY + radius,
        centerX - radius, centerY - radius,
        centerX + radius, centerY - radius,
        outlineColor
      );
      // Draw inner triangle slightly smaller
      tft.fillTriangle(
        centerX, centerY + radius - 1,
        centerX - radius + 1, centerY - radius + 1,
        centerX + radius - 1, centerY - radius + 1,
        pieceColor
      );
      break;

    case ROOK:
      // Draw outline square
      tft.fillRect(centerX - radius, centerY - radius, radius * 2, radius * 2, outlineColor);
      // Draw inner square slightly smaller
      tft.fillRect(centerX - radius + 1, centerY - radius + 1, (radius * 2) - 2, (radius * 2) - 2, pieceColor);
      break;

    case QUEEN:
      // Draw outline circle
      tft.fillCircle(centerX, centerY, radius, outlineColor);
      // Draw inner circle slightly smaller
      tft.fillCircle(centerX, centerY, radius - 1, pieceColor);
      // Draw the dot in the center
      tft.fillCircle(centerX, centerY, radius / 2, bgColor);
      break;

    case KING:
      // Draw outline circle
      tft.fillCircle(centerX, centerY, radius, outlineColor);
      // Draw inner circle slightly smaller
      tft.fillCircle(centerX, centerY, radius - 1, pieceColor);
      // Draw the cross
      tft.drawLine(centerX - radius / 2, centerY, centerX + radius / 2, centerY, bgColor);
      tft.drawLine(centerX, centerY - radius / 2, centerX, centerY + radius / 2, bgColor);
      break;

    default:
      // Empty square, nothing to draw
      break;
  }
}

void handleInput() {
  // Read current button states
  bool currLeftState = digitalRead(leftButton);
  bool currRightState = digitalRead(rightButton);
  bool currUpState = digitalRead(upButton);
  bool currDownState = digitalRead(downButton);
  bool currAState = digitalRead(aButton);
  bool currBState = digitalRead(bButton);

  // Move cursor left
  if (currLeftState == LOW && prevLeftState == HIGH) {
    cursorX = (cursorX - 1 + 8) % 8;
    drawBoard();
  }

  // Move cursor right
  if (currRightState == LOW && prevRightState == HIGH) {
    cursorX = (cursorX + 1) % 8;
    drawBoard();
  }

  // Move cursor up
  if (currUpState == LOW && prevUpState == HIGH) {
    cursorY = (cursorY - 1 + 8) % 8;
    drawBoard();
  }

  // Move cursor down
  if (currDownState == LOW && prevDownState == HIGH) {
    cursorY = (cursorY + 1) % 8;
    drawBoard();
  }

  // Select piece or move
  if (currAState == LOW && prevAState == HIGH) {
    if (selectedX == -1 && selectedY == -1) {
      // No piece selected, try to select a piece
      if (board[cursorY][cursorX].type != EMPTY && board[cursorY][cursorX].color == currentPlayer) {
        selectedX = cursorX;
        selectedY = cursorY;
        drawBoard();
      }
    } else {
      // Piece selected, try to move to cursor position
      if (isLegalMove(selectedX, selectedY, cursorX, cursorY)) {
        movePiece(selectedX, selectedY, cursorX, cursorY);
        resetSelection();
        switchPlayer();
        drawBoard();
        // Check for game over
        checkGameOver();
      }
    }
  }

  // Cancel selection
  if (currBState == LOW && prevBState == HIGH) {
    resetSelection();
    drawBoard();
  }

  // Update previous states
  prevLeftState = currLeftState;
  prevRightState = currRightState;
  prevUpState = currUpState;
  prevDownState = currDownState;
  prevAState = currAState;
  prevBState = currBState;
}

void resetSelection() {
  selectedX = -1;
  selectedY = -1;
}

void switchPlayer() {
  currentPlayer = (currentPlayer == WHITE) ? BLACK : WHITE;
}

void movePiece(int fromX, int fromY, int toX, int toY) {
  // Move the piece
  board[toY][toX] = board[fromY][fromX];
  board[fromY][fromX].type = EMPTY;
  board[fromY][fromX].color = NONE;
}

bool isLegalMove(int fromX, int fromY, int toX, int toY) {
  // Check if target square is within the board
  if (!isWithinBoard(toX, toY)) {
    return false;
  }

  // Check if moving to the same square
  if (fromX == toX && fromY == toY) {
    return false;
  }

  Piece piece = board[fromY][fromX];
  Piece target = board[toY][toX];

  // Cannot capture own piece
  if (target.color == piece.color) {
    return false;
  }

  int dx = toX - fromX;
  int dy = toY - fromY;

  switch (piece.type) {
    case PAWN:
      {
        // Pawns move forward (direction depends on color)
        int direction = (piece.color == WHITE) ? -1 : 1;

        // Normal move
        if (dx == 0 && dy == direction && target.type == EMPTY) {
          return true;
        }

        // Capture move
        if ((dx == -1 || dx == 1) && dy == direction && target.type != EMPTY && target.color != piece.color) {
          return true;
        }

        // Double move from starting position
        if ((fromY == 6 && piece.color == WHITE || fromY == 1 && piece.color == BLACK) &&
            dx == 0 && dy == 2 * direction && target.type == EMPTY && board[fromY + direction][fromX].type == EMPTY) {
          return true;
        }

        return false;
      }

    case KNIGHT:
      if ((abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2)) {
        return true;
      }
      return false;

    case BISHOP:
      if (abs(dx) == abs(dy) && isPathClear(fromX, fromY, toX, toY)) {
        return true;
      }
      return false;

    case ROOK:
      if ((dx == 0 || dy == 0) && isPathClear(fromX, fromY, toX, toY)) {
        return true;
      }
      return false;

    case QUEEN:
      if ((abs(dx) == abs(dy) || dx == 0 || dy == 0) && isPathClear(fromX, fromY, toX, toY)) {
        return true;
      }
      return false;

    case KING:
      if (abs(dx) <= 1 && abs(dy) <= 1) {
        return true;
      }
      // Castling is omitted for simplicity
      return false;

    default:
      return false;
  }
}

bool isPathClear(int fromX, int fromY, int toX, int toY) {
  int dx = (toX - fromX);
  int dy = (toY - fromY);

  int stepX = (dx == 0) ? 0 : (dx / abs(dx));
  int stepY = (dy == 0) ? 0 : (dy / abs(dy));

  int x = fromX + stepX;
  int y = fromY + stepY;

  while (x != toX || y != toY) {
    if (board[y][x].type != EMPTY) {
      return false;
    }
    x += stepX;
    y += stepY;
  }
  return true;
}

bool isWithinBoard(int x, int y) {
  return x >= 0 && x < 8 && y >= 0 && y < 8;
}

void checkGameOver() {
  bool whiteKingPresent = false;
  bool blackKingPresent = false;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (board[y][x].type == KING) {
        if (board[y][x].color == WHITE) {
          whiteKingPresent = true;
        } else if (board[y][x].color == BLACK) {
          blackKingPresent = true;
        }
      }
    }
  }

  if (!whiteKingPresent || !blackKingPresent) {
    // Game over
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    if (!whiteKingPresent) {
      tft.drawString("Black Wins!", tft.width() / 2, tft.height() / 2);
    } else {
      tft.drawString("White Wins!", tft.width() / 2, tft.height() / 2);
    }
    // Wait for a while and then return to menu
    delay(5000);
    // Optionally, reset the game
    chessSetup();
  }
}

void chessLoop() {
  handleInput();
  // Delay to debounce buttons
  delay(100);
}