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
typedef struct {
  PieceType type;
  PlayerColor color;
  bool hasMoved;
} Piece;

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



/**
 * @brief Initializes the chess game by setting up the board and pieces.
 */
void chessSetup();

/**
 * @brief Draws the entire chess board on the display.
 */
void drawBoard();

/**
 * @brief Draws a specific chess piece on the board.
 * 
 * @param piece The chess piece to draw.
 * @param posX The x-coordinate on the display where the piece should be drawn.
 * @param posY The y-coordinate on the display where the piece should be drawn.
 * @param squareSize The size of the square in pixels.
 * @param pieceColor The color of the piece (TFT color).
 * @param bgColor The background color of the square (TFT color).
 */
void drawPiece(Piece piece, int posX, int posY, int squareSize, uint16_t pieceColor, uint16_t bgColor);

/**
 * @brief Handles user input for moving the cursor, selecting pieces, and making moves.
 */
void handleInput();

/**
 * @brief Resets the current piece selection.
 */
void resetSelection();

/**
 * @brief Switches the current player from WHITE to BLACK or vice versa.
 */
void switchPlayer();

/**
 * @brief Moves a piece from one position to another on the board.
 * 
 * @param fromX The x-coordinate of the piece's original position.
 * @param fromY The y-coordinate of the piece's original position.
 * @param toX The x-coordinate of the piece's destination.
 * @param toY The y-coordinate of the piece's destination.
 */
void movePiece(int fromX, int fromY, int toX, int toY);

/**
 * @brief Determines if a move from one position to another is legal according to chess rules.
 * 
 * @param fromX The x-coordinate of the piece's original position.
 * @param fromY The y-coordinate of the piece's original position.
 * @param toX The x-coordinate of the piece's destination.
 * @param toY The y-coordinate of the piece's destination.
 * @return true If the move is legal.
 * @return false If the move is illegal.
 */
bool isLegalMove(int fromX, int fromY, int toX, int toY);

/**
 * @brief Checks if the current player's king is under attack.
 * 
 * @param color The color of the player to check.
 * @return true If the king is in check.
 * @return false If the king is not in check.
 */
bool isInCheck(PlayerColor color);

/**
 * @brief Determines if a piece at a given position can attack another square.
 * 
 * @param fromX The x-coordinate of the attacking piece.
 * @param fromY The y-coordinate of the attacking piece.
 * @param toX The x-coordinate of the target square.
 * @param toY The y-coordinate of the target square.
 * @return true If the piece can attack the target square.
 * @return false Otherwise.
 */
bool isAttacking(int fromX, int fromY, int toX, int toY);

/**
 * @brief Checks if the path between two squares is clear (no pieces in between).
 * 
 * @param fromX The x-coordinate of the starting square.
 * @param fromY The y-coordinate of the starting square.
 * @param toX The x-coordinate of the destination square.
 * @param toY The y-coordinate of the destination square.
 * @return true If the path is clear.
 * @return false If there are any pieces blocking the path.
 */
bool isPathClear(int fromX, int fromY, int toX, int toY);

/**
 * @brief Determines if a given position is within the bounds of the chess board.
 * 
 * @param x The x-coordinate to check.
 * @param y The y-coordinate to check.
 * @return true If the position is within the board.
 * @return false Otherwise.
 */
bool isWithinBoard(int x, int y);

/**
 * @brief Checks if the game has ended due to checkmate or stalemate and handles the endgame.
 */
void checkGameOver();

/**
 * @brief Determines if the current player is in checkmate.
 * 
 * @param color The color of the player to check.
 * @return true If the player is in checkmate.
 * @return false Otherwise.
 */
bool isInCheckmate(PlayerColor color);

/**
 * @brief Determines if the current player is in stalemate.
 * 
 * @param color The color of the player to check.
 * @return true If the player is in stalemate.
 * @return false Otherwise.
 */
bool isInStalemate(PlayerColor color);

/**
 * @brief The main loop function for the chess game, handling input and game progression.
 */
void chessLoop();

#endif // CHESS_H