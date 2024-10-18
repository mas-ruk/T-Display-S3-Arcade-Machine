// tetris.ino

#include "Tetris.h"
#include <SPI.h>
#include <Arduino.h>

// Extern declarations for score arrays and functions
extern ScoreEntry tetrisScores[5];
extern void insertNewScore(ScoreEntry scores[], int newScore);
extern void writeScoresToSD(const char* filename, ScoreEntry scores[]);

// Add the definition for GetNextPosRot
void GetNextPosRot(Point* pnext_pos, int* pnext_rot);

uint16_t BlockImage[8][12][12];                            // Block
uint16_t backBuffer[220][110];                             // GAME AREA
const int Length = 11;     // the number of pixels for a side of a block
const int Width  = 10;     // the number of horizontal blocks
const int Height = 20;     // the number of vertical blocks
int screen[Width][Height] = {0}; // it shows color-numbers of all positions
Point pos; Block block;
int rot, fall_cnt = 0;
bool started = false, gameover = false;
boolean but_A = false, but_LEFT = false, but_RIGHT = false;
boolean but_DOWN = false, but_UP = false;
int game_speed = 20; // 25msec
Block blocks[7] = {
  {{{{-1,0},{0,0},{1,0},{2,0}},{{0,-1},{0,0},{0,1},{0,2}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,1},
  {{{{0,-1},{1,-1},{0,0},{1,0}},{{0,0},{0,0},{0,0},{0,0}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},1,2},
  {{{{-1,-1},{-1,0},{0,0},{1,0}},{{-1,1},{0,1},{0,0},{0,-1}},
  {{-1,0},{0,0},{1,0},{1,1}},{{1,-1},{0,-1},{0,0},{0,1}}},4,3},
  {{{{-1,0},{0,0},{0,1},{1,1}},{{0,-1},{0,0},{-1,0},{-1,1}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,4},
  {{{{-1,0},{0,0},{1,0},{1,-1}},{{-1,-1},{0,-1},{0,0},{0,1}},
  {{-1,1},{-1,0},{0,0},{1,0}},{{0,-1},{0,0},{0,1},{1,1}}},4,5},
  {{{{-1,1},{0,1},{0,0},{1,0}},{{0,-1},{0,0},{1,0},{1,1}},
  {{0,0},{0,0},{0,0},{0,0}},{{0,0},{0,0},{0,0},{0,0}}},2,6},
  {{{{-1,0},{0,0},{1,0},{0,-1}},{{0,-1},{0,0},{0,1},{-1,0}},
  {{-1,0},{0,0},{1,0},{0,1}},{{0,-1},{0,0},{0,1},{1,0}}},4,7}
};
extern uint8_t tetris_img[];
#define GREY 0x5AEB
int pom=0;
int pom2=0;
int pom3=0;
int pom4=0;
int pom5=0; // For upButton

int score=0;
int lvl=1;

void tetrisSetup(void) {
  tft.init();
  tft.setRotation(4); // Adjust as needed
  tft.setTextSize(1); // Adjust text size
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);
  tft.drawLine(11,19,122,19,GREY);
  tft.drawLine(11,19,11,240,GREY);
  tft.drawLine(122,19,122,240,GREY);

  // Clear area for the score and level display
  tft.fillRect(14, 8, 50, 10, TFT_BLACK);
  tft.fillRect(88, 8, 50, 10, TFT_BLACK);

  tft.drawString("SCORE:"+String(score),14,8,1);
  tft.drawString("LVL:"+String(lvl),88,8,1);

  //----------------------------// Make Block ----------------------------
  make_block( 0, TFT_BLACK);        // Type No, Color
  make_block( 1, 0x00F0);       // DDDD     RED
  make_block( 2, 0xFBE4);       // DD,DD    PURPLE
  make_block( 3, 0xFF00);       // D__,DDD  BLUE
  make_block( 4, 0xFF87);       // DD_,_DD  GREEN
  make_block( 5, 0x87FF);       // __D,DDD  YELLOW
  make_block( 6, 0xF00F);       // _DD,DD_  LIGHT GREEN
  make_block( 7, 0xF8FC);       // _D_,DDD  PINK
  //----------------------------------------------------------------------
  PutStartPos();                             // Start Position
  for (int i = 0; i < 4; ++i) screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
  Draw();                                    // Draw block
}

//========================================================================

void tetrisLoop() {
  if (gameover) {
    if(leftButton == 0|| rightButton == 0 || downButton == 0) {
      for (int j = 0; j < Height; ++j)
      for (int i = 0; i < Width; ++i)
        screen[i][j] = 0;
      gameover = false;
      score = 0;
      game_speed = 20;
      lvl = 1;
      PutStartPos();                             // Start Position
      for (int i = 0; i < 4; ++i) screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
      tft.drawString("SCORE:"+String(score),14,8,1);
      tft.drawString("LVL:"+String(lvl),88,8,1);
      Draw();
    }
    return;
  }

  if(gameover == false) {
    Point next_pos;
    int next_rot = rot;
    GetNextPosRot(&next_pos, &next_rot);
    ReviseScreen(next_pos, next_rot);
    delay(game_speed);    // SPEED ADJUST
  }
}

//========================================================================

void GetNextPosRot(Point* pnext_pos, int* pnext_rot) {
  KeyPadLoop();

  if (but_LEFT || but_RIGHT || but_DOWN ) started = true;
  if (!started) return;

  pnext_pos->X = pos.X;
  pnext_pos->Y = pos.Y;

  int fall_interval = 10;  // Default fall speed

  if (but_DOWN) {
    fall_interval = 1;  // Increase drop speed when down button is pressed
  }

  if ((fall_cnt = (fall_cnt + 1) % fall_interval) == 0) {
    pnext_pos->Y += 1;
  }

  if (but_LEFT) { but_LEFT = false; pnext_pos->X -= 1;}
  else if (but_RIGHT) { but_RIGHT = false; pnext_pos->X += 1;}
  else if (but_A) { but_A = false;
    *pnext_rot = (*pnext_rot + block.numRotate - 1)%block.numRotate; 
  }
  else if (but_UP) {
    but_UP = false;
    // Instant drop
    while (true) {
      Point temp_pos = *pnext_pos;
      temp_pos.Y += 1;
      Point temp_squares[4];
      if (GetSquares(block, temp_pos, *pnext_rot, temp_squares)) {
        pnext_pos->Y += 1;
      } else {
        // Can't move further down
        break;
      }
    }
  }
}
//========================================================================
void Draw() {                               // Draw 120x240 in the center
  for (int i = 0; i < Width; ++i) for (int j = 0; j < Height; ++j)
   for (int k = 0; k < Length; ++k) for (int l = 0; l < Length; ++l)
    backBuffer[j * Length + l][i * Length + k] = BlockImage[screen[i][j]][k][l];
    tft.pushImage(12, 20, 110, 220,*backBuffer);
}
//========================================================================
void PutStartPos() {
  game_speed=20;
  pos.X = 4; pos.Y = 1;
  block = blocks[random(7)];
  rot = random(block.numRotate);
}
//========================================================================
bool GetSquares(Block block, Point pos, int rot, Point* squares) {
  bool overlap = false;
  for (int i = 0; i < 4; ++i) {
    Point p;
    p.X = pos.X + block.square[rot][i].X;
    p.Y = pos.Y + block.square[rot][i].Y;
    overlap |= p.X < 0 || p.X >= Width || p.Y < 0 || p.Y >= 
      Height || screen[p.X][p.Y] != 0;
    squares[i] = p;
  }
  return !overlap;
}
//========================================================================

void GameOver() {
  // Update high scores if current score qualifies
  if (score > tetrisScores[4].score) {
    // Insert the new score into the list
    insertNewScore(tetrisScores, score);
    // Write updated scores to SD card
    writeScoresToSD("/tetris_scores.txt", tetrisScores);
  }

  for (int i = 0; i < Width; ++i)
    for (int j = 0; j < Height; ++j)
      if (screen[i][j] != 0) screen[i][j] = 4;
  gameover = true;
}
//========================================================================
void ClearKeys() { but_A=false; but_LEFT=false; but_RIGHT=false; but_UP=false; but_DOWN=false; }
//========================================================================

void KeyPadLoop() {
  // Read left button
  if (leftButton == 0 && rightButton == 1) {
    if (pom == 0) {
      pom = 1;
      ClearKeys();
      but_LEFT = true;
    }
  } else { pom = 0; }

  // Read right button
  if (rightButton == 0 && leftButton == 1) {
    if (pom2 == 0) {
      pom2 = 1;
      ClearKeys();
      but_RIGHT = true;
    }
  } else { pom2 = 0; }

  // Rotate button (assuming it's connected to aButton)
  if (aButton == 0) {
    if (pom4 == 0) {
      pom4 = 1;
      ClearKeys();
      but_A = true;
    }
  } else { pom4 = 0; }

  // Down button (held)
  if (downButton == 0) {
    but_DOWN = true;
  } else {
    but_DOWN = false;
  }

  // Up button (press detection)
  if (upButton == 0) {
    if (pom5 == 0) {
      pom5 = 1;
      ClearKeys();
      but_UP = true;
    }
  } else { pom5 = 0; }
}


//========================================================================
void DeleteLine() {
  for (int j = 0; j < Height; ++j) {
    bool Delete = true;
    for (int i = 0; i < Width; ++i) if (screen[i][j] == 0) Delete = false;
    if (Delete)
    {
       score++;
       if(score%5==0)
        {
        lvl++;
        game_speed=game_speed-4;
        tft.fillRect(88, 8, 50, 10, TFT_BLACK); 
        tft.drawString("LVL:"+String(lvl),88,8,1);
        }
      tft.fillRect(14, 8, 50, 10, TFT_BLACK);
      tft.drawString("SCORE:"+String(score),14,8,1);
          for (int k = j; k >= 1; --k) 
              {
    for (int i = 0; i < Width; ++i)
    {
      screen[i][k] = screen[i][k - 1];
    }
  }
}}}
//========================================================================
void ReviseScreen(Point next_pos, int next_rot) {
  if (!started) return;
  Point next_squares[4];
  // Remove the block from the screen
  for (int i = 0; i < 4; ++i) screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = 0;

  if (GetSquares(block, next_pos, next_rot, next_squares)) {
    // Move the block to the new position
    for (int i = 0; i < 4; ++i){
      screen[next_squares[i].X][next_squares[i].Y] = block.color;
    }
    pos = next_pos;
    rot = next_rot;
  } else {
    // Can't move the block to next_pos, so put it back to current position
    for (int i = 0; i < 4; ++i)
      screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;

    // If the attempted move was down, and we can't move further down
    if (next_pos.Y != pos.Y || but_UP) {
      DeleteLine(); PutStartPos();

      // Check for game over condition
      Point temp_squares[4];
      if (!GetSquares(block, pos, rot, temp_squares)) {
        for (int i = 0; i < 4; ++i)
          screen[pos.X + block.square[rot][i].X][pos.Y + block.square[rot][i].Y] = block.color;
        GameOver();
      }
    }
  }
  Draw();
}
//========================================================================
void make_block( int n , uint16_t color ){            // Make Block color       
  for ( int i =0 ; i < 12; i++ ) for ( int j =0 ; j < 12; j++ ){
    BlockImage[n][i][j] = color;                           // Block color
    if ( i == 0 || j == 0 ) BlockImage[n][i][j] = 0;       // TFT_BLACK Line
  } 
}
//========================================================================