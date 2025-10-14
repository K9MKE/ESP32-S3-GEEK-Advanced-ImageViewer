/*****************************************************************************
* | File        :   LCD_Driver.h
* | Author      :   Waveshare team
* | Function    :   LCD driver
******************************************************************************/
#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include "DEV_Config.h"

#define LCD_WIDTH   135 //LCD width
#define LCD_HEIGHT  240 //LCD height

// Colors
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

void LCD_WriteData_Byte(UBYTE da); 
void LCD_WriteData_Word(UWORD da);
void LCD_WriteReg(UBYTE da);
void LCD_SetCursor(UWORD x1, UWORD y1, UWORD x2,UWORD y2);
void LCD_SetUWORD(UWORD x, UWORD y, UWORD Color);
void LCD_Init(void);
void LCD_SetBacklight(UWORD Value);
void LCD_Clear(UWORD Color);
void LCD_ClearWindow(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color);
void LCD_DrawPoint(UWORD X,UWORD Y,UWORD Color);

#endif