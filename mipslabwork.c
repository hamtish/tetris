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

int blockv[4] = {15,0,0,0};  

int blockv2[2][4] = {{15,0,0,0},         // 24
                    {240,0,0,0}};        // 13

int btnpressed = 0;

int gamepaused = 0;

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

  IFS(0) = 0;

  TMR2 = 0;               // set clock to 0
  PR2 = IPerSec / 50;     // roll over value
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

  display_image(0, screen);
  return;
}

/* This function is called repetitively from the main program */
void labwork( void )
{
  if (gamepaused)
    return;

  display_image(0, screen); // update image on screen

  if (IFS(0)) {
    IFS(0) = 0;
    movedownlogic();
  } 

  int btnvalue = getbtns();
  if (btnvalue != 0 && btnpressed == 0) {
    btnpressed = 1;
    if ((btnvalue & 0x01) == 1 && (pos < 384 || n == 15)) 
      rightbtnpressed();
    
    if ((btnvalue & 0x02) == 2 && (pos > 127 || n == 240))
      leftbtnpressed();
    
    if ((btnvalue & 0x04) == 4) {
      display_string(0, "Game stopped");
      display_string(1, "");
      display_string(2, "");	
      display_update();
      gamepaused = 1;
    }
  }
  else if (btnvalue == 0)
    btnpressed = 0;
}

/* every tick the block moves down and checks if tetris */
void movedownlogic( void ) {
  if (screen[pos-1] == 0) {
    screen[pos+3] = 0;
    screen[--pos] = n;
  }
  else if (screen[pos-1] == 255 || screen[pos-1] == n) {
    tetris();
    newblock();
  }
  else {
    if (screen[pos+3] == n) 
      screen[pos+3] = 0;
    else
      screen[pos+3] = (255 & ~n);
  
    screen[--pos] = 255;
  }
}

/* check if tetris -> remove blocks */
void tetris( void ) {
  int i;
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
}

/* add new block at the top of the screen */
void newblock( void ) {
  int blocktype = 0;//(pos+n)%4;  // value between [0-3]
  pos = 245;
  int i;
  for (i = 0; i < 4; i++)     // 
    screen[pos+i] = 255;      // **

  if (blocktype == 0) 
    for (i = 0; i < 4; i++)   //
      screen[pos+i] = 15;     // *
  
  else if (blocktype == 2) 
    for (i = 4; i < 8; i++)   // *
      screen[pos+i] = 15;     // **
  
  else if (blocktype == 3) 
    for (i = 4; i < 8; i++)   // **
      screen[pos+i] = 255;    // **
}

/* button 1 is pressed -> move block to the right */
void rightbtnpressed( void ) {
  if (n == 15) {
    if (screen[pos] == 255)
      return;

    n = 240;
  }
  else {
    if (screen[pos+128] & 0xf)
      return;

    n = 15;
    screen[pos] -= 240;
    screen[pos+1] -= 240;
    screen[pos+2] -= 240;
    screen[pos+3] -= 240;
    pos += 128;
  }
  screen[pos] += n;
  screen[pos+1] += n;
  screen[pos+2] += n;
  screen[pos+3] += n;
}

/* button 2 is pressed -> move block to the left */
void leftbtnpressed( void ) {
  if (n == 240) {
    if (screen[pos] == 255)
      return;

    n = 15;
  }
  else {
    if (screen[pos-128] & 0xf0)
      return;

    n = 240;
    screen[pos] -= 15;
    screen[pos+1] -= 15;
    screen[pos+2] -= 15;
    screen[pos+3] -= 15;
    pos -= 128;
  }
  screen[pos] += n;
  screen[pos+1] += n;
  screen[pos+2] += n;
  screen[pos+3] += n;
}