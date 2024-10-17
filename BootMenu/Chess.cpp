#include "Chess.h"

// Definition of global variables
Piece board[8][8];
PlayerColor currentPlayer = WHITE;
int cursorX = 0;
int cursorY = 0;
int selectedX = -1;
int selectedY = -1;
int prevLeftState = 1;
int prevRightState = 1;
int prevUpState = 1;
int prevDownState = 1;
int prevAState = 1;
int prevBState = 1;
int swapInt = 0;
int currLeftState = leftButton;
int currRightState = rightButton;
int currUpState = upButton;
int currDownState = downButton;
int currAState = aButton;
int currBState = bButton;

// Variables for en passant
int enPassantX = -1;
int enPassantY = -1;

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
      if (pieceColor == TFT_WHITE) {
        tft.fillCircle(centerX, centerY, radius / 2, TFT_BLACK);
      } else if (pieceColor == TFT_BLACK) {
        tft.fillCircle(centerX, centerY, radius / 2, TFT_WHITE);
      }
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
  if (swapInt == 0) {
    currLeftState = leftButton;
    currRightState = rightButton;
    currUpState = upButton;
    currDownState = downButton;
    currAState = aButton;
    currBState = bButton;
  } if (swapInt == 1) {
    currLeftState = leftButton2;
    currRightState = rightButton2;
    currUpState = upButton2;
    currDownState = downButton2;
    currAState = aButton2;
    currBState = bButton2;
  }

  // Move cursor left
  if (currLeftState == 0 && prevLeftState == 1) {
    cursorX = (cursorX - 1 + 8) % 8;
    drawBoard();
  }

  // Move cursor right
  if (currRightState == 0 && prevRightState == 1) {
    cursorX = (cursorX + 1) % 8;
    drawBoard();
  }

  // Move cursor up
  if (currUpState == 0 && prevUpState == 1) {
    cursorY = (cursorY - 1 + 8) % 8;
    drawBoard();
  }

  // Move cursor down
  if (currDownState == 0 && prevDownState == 1) {
    cursorY = (cursorY + 1) % 8;
    drawBoard();
  }

  // Select piece or move
  if (currAState == 0 && prevAState == 1) {
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
  if (currBState == 0 && prevBState == 1) {
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
  swapInt = ((swapInt + 1) % 2);
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

  // Check if it's this player's turn
  if (piece.color != currentPlayer) {
    return false;
  }

  // Check piece-specific movement rules
  bool validMove = false;

  switch (piece.type) {
    case PAWN: {
      int direction = (piece.color == WHITE) ? -1 : 1;

      // Normal move
      if (dx == 0 && dy == direction && target.type == EMPTY) {
        validMove = true;
      }

      // Double move from starting position
      if (!piece.hasMoved && dx == 0 && dy == 2 * direction && target.type == EMPTY && board[fromY + direction][fromX].type == EMPTY) {
        validMove = true;
      }

      // Capture move
      if ((dx == -1 || dx == 1) && dy == direction && target.type != EMPTY && target.color != piece.color) {
        validMove = true;
      }

      // En passant
      if ((dx == -1 || dx == 1) && dy == direction && target.type == EMPTY) {
        if (enPassantX == toX && enPassantY == toY) {
          validMove = true;
        }
      }

      break;
    }
    case KNIGHT:
      if ((abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2)) {
        validMove = true;
      }
      break;
    case BISHOP:
      if (abs(dx) == abs(dy) && isPathClear(fromX, fromY, toX, toY)) {
        validMove = true;
      }
      break;
    case ROOK:
      if ((dx == 0 || dy == 0) && isPathClear(fromX, fromY, toX, toY)) {
        validMove = true;
      }
      break;
    case QUEEN:
      if ((abs(dx) == abs(dy) || dx == 0 || dy == 0) && isPathClear(fromX, fromY, toX, toY)) {
        validMove = true;
      }
      break;
    case KING:
      if (abs(dx) <= 1 && abs(dy) <= 1) {
        validMove = true;
      }
      // Castling
      if (!piece.hasMoved && dy == 0 && (dx == 2 || dx == -2)) {
        int rookX = (dx == 2) ? 7 : 0;
        Piece rook = board[fromY][rookX];
        if (rook.type == ROOK && rook.color == piece.color && !rook.hasMoved) {
          if (isPathClear(fromX, fromY, rookX, fromY)) {
            // Ensure squares king passes through are not under attack
            int step = dx / abs(dx);
            for (int i = 1; i <= abs(dx); i++) {
              int x = fromX + i * step;
              // Temporarily move the king to each square to check for attacks
              Piece originalFrom = board[fromY][fromX];
              Piece originalTo = board[fromY][x];
              board[fromY][x] = piece;
              board[fromY][fromX].type = EMPTY;
              board[fromY][fromX].color = NONE;
              board[fromY][fromX].hasMoved = false;

              bool inCheck = isInCheck(currentPlayer);

              // Undo the move
              board[fromY][fromX] = originalFrom;
              board[fromY][x] = originalTo;

              if (inCheck) {
                return false;
              }
            }
            validMove = true;
          }
        }
      }
      break;
    default:
      return false;
  }

  if (!validMove) {
    return false;
  }

  // Simulate the move to check if it puts own king in check
  Piece originalFrom = board[fromY][fromX];
  Piece originalTo = board[toY][toX];

  board[toY][toX] = board[fromY][fromX];
  board[fromY][fromX].type = EMPTY;
  board[fromY][fromX].color = NONE;
  board[fromY][fromX].hasMoved = false;

  bool inCheck = isInCheck(currentPlayer);

  // Undo the move
  board[fromY][fromX] = originalFrom;
  board[toY][toX] = originalTo;

  if (inCheck) {
    return false;
  }

  return true;
}

bool isInCheck(PlayerColor color) {
  // Find the king's position
  int kingX = -1, kingY = -1;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (board[y][x].type == KING && board[y][x].color == color) {
        kingX = x;
        kingY = y;
        break;
      }
    }
    if (kingX != -1) break;
  }

  if (kingX == -1 || kingY == -1) {
    return true; // King not found; game should end
  }

  // Check if any enemy piece can attack the king
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (board[y][x].color != color && board[y][x].color != NONE) {
        if (isAttacking(x, y, kingX, kingY)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool isAttacking(int fromX, int fromY, int toX, int toY) {
  Piece piece = board[fromY][fromX];
  int dx = toX - fromX;
  int dy = toY - fromY;

  switch (piece.type) {
    case PAWN: {
      int direction = (piece.color == WHITE) ? -1 : 1;
      if ((dx == -1 || dx == 1) && dy == direction) {
        return true;
      }
      break;
    }
    case KNIGHT:
      if ((abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2)) {
        return true;
      }
      break;
    case BISHOP:
      if (abs(dx) == abs(dy) && isPathClear(fromX, fromY, toX, toY)) {
        return true;
      }
      break;
    case ROOK:
      if ((dx == 0 || dy == 0) && isPathClear(fromX, fromY, toX, toY)) {
        return true;
      }
      break;
    case QUEEN:
      if ((abs(dx) == abs(dy) || dx == 0 || dy == 0) && isPathClear(fromX, fromY, toX, toY)) {
        return true;
      }
      break;
    case KING:
      if (abs(dx) <= 1 && abs(dy) <= 1) {
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

bool isPathClear(int fromX, int fromY, int toX, int toY) {
  int dx = toX - fromX;
  int dy = toY - fromY;

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

void movePiece(int fromX, int fromY, int toX, int toY) {
  // Move the piece
  board[toY][toX] = board[fromY][fromX];
  board[toY][toX].hasMoved = true;
  board[fromY][fromX].type = EMPTY;
  board[fromY][fromX].color = NONE;
  board[fromY][fromX].hasMoved = false;

  // Check for en passant capture
  if (board[toY][toX].type == PAWN) {
    if (toX == enPassantX && toY == enPassantY) {
      int capturedPawnY = (currentPlayer == WHITE) ? toY + 1 : toY - 1;
      board[capturedPawnY][toX].type = EMPTY;
      board[capturedPawnY][toX].color = NONE;
      board[capturedPawnY][toX].hasMoved = false;
    }
  }

  // Reset en passant variables
  enPassantX = -1;
  enPassantY = -1;

  // Check if pawn moved two squares forward (for en passant)
  if (board[toY][toX].type == PAWN && abs(toY - fromY) == 2) {
    enPassantX = toX;
    enPassantY = (fromY + toY) / 2;
  }

  // Handle castling
  if (board[toY][toX].type == KING && abs(toX - fromX) == 2) {
    if (toX == 6) {
      // Kingside castling
      board[toY][5] = board[toY][7];
      board[toY][5].hasMoved = true;
      board[toY][7].type = EMPTY;
      board[toY][7].color = NONE;
      board[toY][7].hasMoved = false;
    } else if (toX == 2) {
      // Queenside castling
      board[toY][3] = board[toY][0];
      board[toY][3].hasMoved = true;
      board[toY][0].type = EMPTY;
      board[toY][0].color = NONE;
      board[toY][0].hasMoved = false;
    }
  }

  // Handle pawn promotion
  if (board[toY][toX].type == PAWN && (toY == 0 || toY == 7)) {
    // Promote pawn to queen
    board[toY][toX].type = QUEEN;
  }
}

void checkGameOver() {
  if (isInCheckmate(currentPlayer)) {
    // Game over
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    if (currentPlayer == WHITE) {
      tft.drawString("Black Wins!", tft.width() / 2, tft.height() / 2);
    } else {
      tft.drawString("White Wins!", tft.width() / 2, tft.height() / 2);
    }
    delay(5000);
    chessSetup();
  } else if (isInStalemate(currentPlayer)) {
    // Stalemate
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Stalemate!", tft.width() / 2, tft.height() / 2);
    delay(5000);
    chessSetup();
  }
}

bool isInCheckmate(PlayerColor color) {
  if (!isInCheck(color)) {
    return false;
  }
  // Check if the player has any legal moves
  for (int fromY = 0; fromY < 8; fromY++) {
    for (int fromX = 0; fromX < 8; fromX++) {
      if (board[fromY][fromX].color == color) {
        for (int toY = 0; toY < 8; toY++) {
          for (int toX = 0; toX < 8; toX++) {
            if (isLegalMove(fromX, fromY, toX, toY)) {
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}

bool isInStalemate(PlayerColor color) {
  if (isInCheck(color)) {
    return false;
  }
  // Check if the player has any legal moves
  for (int fromY = 0; fromY < 8; fromY++) {
    for (int fromX = 0; fromX < 8; fromX++) {
      if (board[fromY][fromX].color == color) {
        for (int toY = 0; toY < 8; toY++) {
          for (int toX = 0; toX < 8; toX++) {
            if (isLegalMove(fromX, fromY, toX, toY)) {
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}

void chessLoop() {
  handleInput();
  // No need to delay here; handleInput should handle button debouncing
}
