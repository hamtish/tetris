/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog 

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

#define Fosc 80000000
#define PreScalar 256
#define IPerSec Fosc / PreScalar

uint8_t screen[128*4] = {0};
uint8_t screen2[128*4] = {0};
int pos = 240;
int n = 15;

int blocktype = 0;

int blockv[4] = {15,0,0,0};  
int blockv2[4] = {0,0,0,0};

int btnpressed = 0;

int gamepaused = 0;

int speed = 50;

int rand = 0;

int i;

char score[10] = {8,8,8,8,8,8,8,48,48,0};
int scoreint = 0;
int highscore1 = 0;
int highscore2 = 0;
int highscore3 = 0;

int gamestate = -1; // -1 - title, 0 - startscreen, 1 - running, 2 - game over, 3 - highscore

char textstring[] = "text, more text, and even more text!";

/* Interrupt Service Routine */
void user_isr( void )
{
  return;
}

/* Lab-specific initialization goes here */
void labinit( void )
{
  volatile int *trise = (volatile int *) 0xbf886100;
  *trise = *trise & 0xffffff00;

  volatile int *porte = (volatile int *) 0xbf886110;
  *porte = *porte & 0xffffff00;

  TRISD = TRISD & 0x0fe0;

  TRISF |= 1; // btn 4

  IFS(0) = 0;

  TMR2 = 0;               // set clock to 0
  PR2 = IPerSec / speed;     // roll over value
  T2CON = 0x08070;         // 1000 0000 0111 0000    bit 1: external/ internal clock

  // random nr
  TMR3 = 0;               // set clock to 0
  PR3 = 65293;            // roll over value
  T3CON = 0x08070;        // 1000 0000 0111 0000    bit 1: external/ internal clock

  // line at the bottom of the screen
  screen[0] = 255;
  screen[128] = 255;
  screen[256] = 255;
  screen[384] = 255;

  screen[pos] = 15;
  screen[pos+1] = 15;
  screen[pos+2] = 15;
  screen[pos+3] = 15;

  display_string(2, "     TETRIS");
  display_update();
  
  return;
}

/* This function is called repetitively from the main program */
void labwork( void )
{
  int btnvalue = getbtns();
  if (btnvalue != 0 && btnpressed == 0) {
    btnpressed = 1;

    if (gamestate == -1) {
      gamestate = 0;
      // start screen
      display_string(0, "  BTN 1: ROTATE");
      display_string(1, "  BTN 2: RIGHT");
      display_string(2, "  BTN 3: PAUSE");
      display_string(3, "  BTN 4: LEFT");
      display_update();
      return;
    }

    if (gamestate == 0) {
      countdown();
      gamestate = 1;
      return;
    }
    else if (gamestate == 2) {
      gamestate = 3;
      showhighscores();
      return;
    }
    else if (gamestate == 3) {
      restart();
      return;
    }

    if ((btnvalue & 0x04) == 4) {
      display_string(0, "");
      display_string(1, "");
      display_string(2, "   GAME PAUSED");
      display_string(3, "");
      display_update();
      gamepaused = (gamepaused+1)%2;
    }
    if (gamepaused)
        return;

    if ((btnvalue & 0x01) == 1)
      rotateblock();

    if ((btnvalue & 0x02) == 2 && (pos < 384 || n == 15)) 
      rightbtnpressed();
    
    if ((btnvalue & 0x08) == 8 && (pos > 127 || n == 240))
      leftbtnpressed();
  }
  else if (btnvalue == 0)
    btnpressed = 0;
  
  if (gamepaused || gamestate != 1)
    return;

  display_image(0, screen); // update image on screen

  if (IFS(0)) {
    IFS(0) = 0;
    movedownlogic();
  } 
}

/* every tick the block moves down and checks if tetris */
void movedownlogic( void ) {
  if ( (screen[pos-1] & blockv[0]) || (screen[pos-1+128] & blockv[2]) ||
       ((screen[pos+3] & ~blockv[0]) & blockv[1]) || ((screen[pos+3+128] & ~blockv[2]) & blockv[3]) ) {

    if (pos%128 > 107) {
      gamestate = 2;
      display_string(0, "    GAME OVER");
      display_string(1, "     score:");
      display_string(3, "");
      score[8] = scoreint%10 + 48;
      score[7] = scoreint/10 + 48;
      display_string(2, score);	
      display_update();
      delay(150000);
    }

    tetris();
    newblock();
  }
  else {
    // clear block
    for (i = 0; i < 4; i++) {
      screen[pos+i] -= blockv[0];
      screen[pos+4+i] -= blockv[1];
      screen[pos+128+i] -= blockv[2];
      screen[pos+128+4+i] -= blockv[3];
    }
    // add block
    pos--;
    for (i = 0; i < 4; i++) {
      screen[pos+i] += blockv[0];
      screen[pos+4+i] += blockv[1];
      screen[pos+128+i] += blockv[2];
      screen[pos+128+4+i] += blockv[3];
    }
  }
}

/* check if tetris -> remove blocks */
void tetris( void ) {
  for (i = 0; i < 4; i++)
    if (screen[(pos%128 + i*128)] != 255) {
      return;         // no tetris -> return function
    }
  
  for (i = 0; i < 4; i++) {
    screen[pos%128 + i*128] = 0;    // tetris -> remove blocks
    screen[(pos+1)%128 + i*128] = 0;
    screen[(pos+2)%128 + i*128] = 0;
    screen[(pos+3)%128 + i*128] = 0;
  }
  // move everything over tetris row one down
  for (i = pos%128; i < 124; i++) {
    screen2[i] = screen[i+4];
    screen2[i+128] = screen[i+4+128];
    screen2[i+128*2] = screen[i+4+128*2];
    screen2[i+128*3] = screen[i+4+128*3];
  }
  for (i = pos%128; i < 124; i++) {
    screen[i] = screen2[i];
    screen[i+128] = screen2[i+128];
    screen[i+128*2] = screen2[i+128*2];
    screen[i+128*3] = screen2[i+128*3];
  }

  scoreint += 1;

  // increase speed
  speed += 2;
  PR2 = IPerSec / speed;

  tetris();
}

/* add new block at the top of the screen */
void newblock( void ) {
  blocktype = TMR3%4;  // value between [0-3]
  pos = 240;
  n = 15;
  blockv[0] = 255;
  blockv[1] = 0;
  blockv[2] = 0;
  blockv[3] = 0;
  if ( blocktype == 0 )
    blockv[0] = 15;
  else if( blocktype == 2 )
    blockv[1] = 15;
  else if( blocktype == 3 )
    blockv[1] = 255;

  for (i = 0; i < 4; i++) {
    screen[pos+i] = blockv[0];
    screen[pos+4+i] = blockv[1];
  }
}

/* button 1 is pressed -> move block to the right */
void rightbtnpressed( void ) {
  if (pos >= 384 && (blocktype && !(blockv[0] == 15 && blockv[1] == 15)))
    return;

  for (i = 0; i < 4; i++)
    blockv2[i] = 0;

  if (n == 15) {
    blockv2[0] = ((blockv[0] & 15) << 4);
    blockv2[1] = ((blockv[1] & 15) << 4);
    blockv2[2] = ((blockv[0] & 240) >> 4) | (blockv[2] << 4);
    blockv2[3] = ((blockv[1] & 240) >> 4) | (blockv[3] << 4);
  }
  else {
    blockv2[2] = (blockv[0] >> 4) | (blockv[2] << 4);
    blockv2[3] = (blockv[1] >> 4) | (blockv[3] << 4);
  }

  if (checklegalmove())
    return;

  if (n == 15)
    for (i = 0; i < 4; i++)
      blockv[i] = blockv2[i];
  else {
    blockv[0] = blockv2[2];
    blockv[1] = blockv2[3];
    blockv[2] = 0;
    blockv[3] = 0;
  }

  if (n == 15) {
    for (i = 0; i < 4; i++) {
      screen[pos+i] += blockv[0];
      screen[pos+4+i] += blockv[1];
      screen[pos+128+i] += blockv[2];
      screen[pos+128+4+i] += blockv[3];
    }
    n = 240;
  }
  else {
    n = 15;
    pos += 128;
    for (i = 0; i < 4; i++) {
      screen[pos+i] += blockv[0];
      screen[pos+4+i] += blockv[1];
    }
  }
}

/* button 2 is pressed -> move block to the left */
void leftbtnpressed( void ) {
  if ( pos < 128 && (n == 15) )
    return;

  for (i = 0; i < 4; i++)
    blockv2[i] = 0;

  if (n == 15) {
    blockv2[0] = ((blockv[0] & 15) << 4);
    blockv2[1] = ((blockv[1] & 15) << 4);
    blockv2[2] = ((blockv[0] & 240) >> 4) | (blockv[2] << 4);
    blockv2[3] = ((blockv[1] & 240) >> 4) | (blockv[3] << 4);
  }
  else {
    blockv2[0] = (blockv[0] >> 4) | (blockv[2] << 4);
    blockv2[1] = (blockv[1] >> 4) | (blockv[3] << 4);
  }

  if (checklegalmove())
    return;

  if (n == 15)
    for (i = 0; i < 4; i++)
      blockv[i] = blockv2[i];
  else {
    blockv[0] = blockv2[0];
    blockv[1] = blockv2[1];
    blockv[2] = 0;
    blockv[3] = 0;
  }

  if (n == 15) {
    pos -= 128;
    for (i = 0; i < 4; i++) {
      screen[pos+i] += blockv[0];
      screen[pos+4+i] += blockv[1];
      screen[pos+128+i] += blockv[2];
      screen[pos+128+4+i] += blockv[3];
    }
    n = 240;
  }
  else {
    n = 15;
    for (i = 0; i < 4; i++) {
      screen[pos+i] += blockv[0];
      screen[pos+4+i] += blockv[1];
    }
  }
}

// rotates the block clock wise
void rotateblock( void ) {
  if (blocktype == 0 || blocktype == 3)
    return;

  for (i = 0; i < 4; i++)
    blockv2[i] = 0;

  if (n == 15) {
    blockv2[0] = ((blockv[0] & 240) >> 4) | (blockv[1] & 240);
    blockv2[1] = ((blockv[1] & 15) << 4) | (blockv[0] & 15);
  }
  else {
    blockv2[0] = blockv[2] << 4;
    blockv2[1] = blockv[0];
    blockv2[2] = blockv[3];
    blockv2[3] = blockv[1] >> 4;
  }

  if (checklegalmove())
    return;

  for (i = 0; i < 4; i++)
    blockv[i] = blockv2[i];

  for (i = 0; i < 4; i++) {
    screen[pos+i] += blockv[0];
    screen[pos+4+i] += blockv[1];
    screen[pos+128+i] += blockv[2];
    screen[pos+128+4+i] += blockv[3];
  }
}

int checklegalmove( void ) {
  // clear block
  for (i = 0; i < 4; i++) {
    screen[pos+i] -= blockv[0];
    screen[pos+4+i] -= blockv[1];
    screen[pos+128+i] -= blockv[2];
    screen[pos+128+4+i] -= blockv[3];
  }

  for (i = 0; i < 4; i++) {
    if ((screen[pos+i] & blockv2[0]) || (screen[pos+4+i] & blockv2[1]) ||
        (screen[pos+128+i] & blockv2[2]) || (screen[pos+128+4+i] & blockv2[3])) {
      // reverse clear block
      for (i = 0; i < 4; i++) {
        screen[pos+i] += blockv[0];
        screen[pos+4+i] += blockv[1];
        screen[pos+128+i] += blockv[2];
        screen[pos+128+4+i] += blockv[3];
      }
      return 1;
    }
  }
  return 0;
}

void showhighscores( void ) {
  display_string(0, "    Highscore");
  char hs1[12] = {8,8,8,8,8,8,49,46,8,48,48,0};
  char hs2[12] = {8,8,8,8,8,8,50,46,8,48,48,0};
  char hs3[12] = {8,8,8,8,8,8,51,46,8,48,48,0};
  if (scoreint > highscore1) {
    highscore3 = highscore2;
    highscore2 = highscore1;
    highscore1 = scoreint;
  }
  else if(scoreint > highscore2) {
    highscore3 = highscore2;
    highscore2 = scoreint;
  }
  else if(scoreint > highscore3)
    highscore3 = scoreint;

  hs1[9] = highscore1/10 + 48;
  hs1[10] = highscore1%10 + 48;
  hs2[9] = highscore2/10 + 48;
  hs2[10] = highscore2%10 + 48;
  hs3[9] = highscore3/10 + 48;
  hs3[10] = highscore3%10 + 48;

  display_string(1, hs1);
  display_string(2, hs2);
  display_string(3, hs3);

  display_update();
}

void restart( void ) {
  for (i = 0; i < 512; i++)
  screen[i] = 0;
  pos = 240;
  n = 15;
  blocktype = 0;
  for (i = 0; i < 4; i++) {
    blockv[i] = 0;
    blockv2[i] = 0;
  }
  blockv[0] = 15;  
  gamepaused = 0;
  speed = 50;
  scoreint = 0;
  gamestate = -1; // -1 - title, 0 - startscreen, 1 - running, 2 - game over, 3 - highscore
  TMR2 = 0;               // set clock to 0
  PR2 = IPerSec / speed;     // roll over value
  T2CON = 0x08070;         // 1000 0000 0111 0000    bit 1: external/ internal clock
  // line at the bottom of the screen
  screen[0] = 255;
  screen[128] = 255;
  screen[256] = 255;
  screen[384] = 255;

  screen[pos] = 15;
  screen[pos+1] = 15;
  screen[pos+2] = 15;
  screen[pos+3] = 15;
  display_string(0, "");
  display_string(1, "");
  display_string(2, "     TETRIS");
  display_string(3, "");
  display_update();
}

void countdown( void ) {
  // countdown
  screen[320] = 31;
  screen[321] = 16;
  screen[322] = 16;
  screen[323] = 16;
  screen[324] = 31;
  screen[325] = 16;
  screen[326] = 16;
  screen[327] = 31;
  display_image(0, screen); // update image on screen
  delay(50000);
  screen[321] = 1;
  screen[322] = 1;
  screen[323] = 1;
  display_image(0, screen); // update image on screen
  delay(50000);
  for (i = 0; i < 8; i++)
    screen[320+i] =16;
  display_image(0, screen); // update image on screen
  delay(50000);
  for (i = 0; i < 8; i++)
    screen[320+i] = 0;
}
