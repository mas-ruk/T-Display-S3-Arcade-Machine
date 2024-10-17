#ifndef CHESS_H
#define CHESS_H

#include <TFT_eSPI.h> // Assumes tft object is globally accessible

// External declarations for global variables
extern TFT_eSPI tft; // Declare TFT object
extern int leftButton; // Declare button variables
extern int rightButton;
extern int upButton;
extern int downButton;
extern int aButton;
extern int bButton;

extern int leftButton2; // Declare button variables p2
extern int rightButton2;
extern int upButton2;
extern int downButton2;
extern int aButton2;
extern int bButton2;

// Define piece types
enum PieceType {
  EMPTY = 0,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING
};

// Define player colors
enum PlayerColor {
  NONE = 0,
  WHITE,
  BLACK
};

// Structure to represent a piece
struct Piece {
  PieceType type;
  PlayerColor color;
};

// External declarations of variables
extern Piece board[8][8];
extern PlayerColor currentPlayer;
extern int cursorX;
extern int cursorY;
extern int selectedX;
extern int selectedY;

extern int prevLeftState;
extern int prevRightState;
extern int prevUpState;
extern int prevDownState;
extern int prevAState;
extern int prevBState;



// Function declarations
void chessSetup();
void chessLoop();
void drawBoard();
void drawPiece(Piece piece, int posX, int posY, int squareSize, uint16_t pieceColor, uint16_t bgColor);
void handleInput();
bool isLegalMove(int fromX, int fromY, int toX, int toY);
bool isWithinBoard(int x, int y);
void movePiece(int fromX, int fromY, int toX, int toY);
void resetSelection();
void switchPlayer();
bool isPathClear(int fromX, int fromY, int toX, int toY);
void checkGameOver();

#endif // CHESS_H