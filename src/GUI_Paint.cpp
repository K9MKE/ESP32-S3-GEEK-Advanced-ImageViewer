/*****************************************************************************
* | File        :   GUI_Paint.cpp
* | Author      :   Waveshare team (modified)
* | Function    : Basic paint functions for image display
******************************************************************************/
#include "GUI_Paint.h"
#include <pgmspace.h>

volatile PAINT Paint;

/******************************************************************************
function: Create a new image cache named ImageName and set the cache space size
parameter:
    Width   :   X direction size
    Height  :   Y direction size  
    Rotate  :   Rotation angle
    Color   :   Initial color
******************************************************************************/
void Paint_NewImage(UWORD Width, UWORD Height, UWORD Rotate, UWORD Color)
{
    Paint.Image = NULL;
    Paint.WidthMemory = Width;
    Paint.HeightMemory = Height;
    Paint.Color = Color;    
    Paint.WidthByte = Width;
    Paint.HeightByte = Height;
    
    Paint.Rotate = Rotate;
    Paint.Mirror = MIRROR_NONE;
    
    if(Rotate == ROTATE_0 || Rotate == ROTATE_180) {
        Paint.Width = Width;
        Paint.Height = Height;
    } else {
        Paint.Width = Height;
        Paint.Height = Width;
    }
}

/******************************************************************************
function: Set the color of a pixel
parameter:
    Xpoint : X coordinate
    Ypoint : Y coordinate
    Color  : color
******************************************************************************/
void Paint_SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color)
{
    if(Xpoint > Paint.Width || Ypoint > Paint.Height) {
        return;
    }
    
    LCD_SetCursor(Xpoint, Ypoint, Xpoint, Ypoint);
    LCD_WriteData_Word(Color);
}

/******************************************************************************
function: Display RGB565 image data
parameter:
    image            : Image data array (RGB565 format)
    xStart           : X starting coordinates
    yStart           : Y starting coordinates
    W_Image          : Image width
    H_Image          : Image height
******************************************************************************/
void Paint_DrawImage(const uint16_t *image, UWORD xStart, UWORD yStart, UWORD W_Image, UWORD H_Image)
{
    UWORD i, j;
    
    LCD_SetCursor(xStart, yStart, xStart + W_Image - 1, yStart + H_Image - 1);
    
    for (j = 0; j < H_Image; j++) {
        for (i = 0; i < W_Image; i++) {
            // Read RGB565 color from PROGMEM
            uint16_t color = pgm_read_word(&image[j * W_Image + i]);
            LCD_WriteData_Word(color);
        }
    }
}