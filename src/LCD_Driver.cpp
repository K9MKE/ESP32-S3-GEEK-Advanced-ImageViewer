/*****************************************************************************
* | File        :   LCD_Driver.cpp
* | Author      :   Waveshare team
* | Function    :   LCD driver
******************************************************************************/
#include "LCD_Driver.h"

void LCD_WriteReg(UBYTE reg)
{
    DEV_Digital_Write(DEV_DC_PIN, 0);
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WRITE(reg);
    DEV_Digital_Write(DEV_CS_PIN, 1);
}

void LCD_WriteData_Byte(UBYTE data)
{
    DEV_Digital_Write(DEV_DC_PIN, 1);
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WRITE(data);
    DEV_Digital_Write(DEV_CS_PIN, 1);
}

void LCD_WriteData_Word(UWORD data)
{
    DEV_Digital_Write(DEV_DC_PIN, 1);
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WRITE((data >> 8) & 0xFF);
    DEV_SPI_WRITE(data & 0xFF);
    DEV_Digital_Write(DEV_CS_PIN, 1);
}

void LCD_Init(void)
{
    // Reset
    DEV_Digital_Write(DEV_RST_PIN, 1);
    DEV_Delay_ms(100);
    DEV_Digital_Write(DEV_RST_PIN, 0);
    DEV_Delay_ms(100);
    DEV_Digital_Write(DEV_RST_PIN, 1);
    DEV_Delay_ms(100);

    // ST7789V init sequence
    LCD_WriteReg(0x36);
    LCD_WriteData_Byte(0x00);

    LCD_WriteReg(0x3A);
    LCD_WriteData_Byte(0x05);

    LCD_WriteReg(0xB2);
    LCD_WriteData_Byte(0x0C);
    LCD_WriteData_Byte(0x0C);
    LCD_WriteData_Byte(0x00);
    LCD_WriteData_Byte(0x33);
    LCD_WriteData_Byte(0x33);

    LCD_WriteReg(0xB7);
    LCD_WriteData_Byte(0x35);

    LCD_WriteReg(0xBB);
    LCD_WriteData_Byte(0x19);

    LCD_WriteReg(0xC0);
    LCD_WriteData_Byte(0x2C);

    LCD_WriteReg(0xC2);
    LCD_WriteData_Byte(0x01);

    LCD_WriteReg(0xC3);
    LCD_WriteData_Byte(0x12);

    LCD_WriteReg(0xC4);
    LCD_WriteData_Byte(0x20);

    LCD_WriteReg(0xC6);
    LCD_WriteData_Byte(0x0F);

    LCD_WriteReg(0xD0);
    LCD_WriteData_Byte(0xA4);
    LCD_WriteData_Byte(0xA1);

    LCD_WriteReg(0xE0);
    LCD_WriteData_Byte(0xD0);
    LCD_WriteData_Byte(0x04);
    LCD_WriteData_Byte(0x0D);
    LCD_WriteData_Byte(0x11);
    LCD_WriteData_Byte(0x13);
    LCD_WriteData_Byte(0x2B);
    LCD_WriteData_Byte(0x3F);
    LCD_WriteData_Byte(0x54);
    LCD_WriteData_Byte(0x4C);
    LCD_WriteData_Byte(0x18);
    LCD_WriteData_Byte(0x0D);
    LCD_WriteData_Byte(0x0B);
    LCD_WriteData_Byte(0x1F);
    LCD_WriteData_Byte(0x23);

    LCD_WriteReg(0xE1);
    LCD_WriteData_Byte(0xD0);
    LCD_WriteData_Byte(0x04);
    LCD_WriteData_Byte(0x0C);
    LCD_WriteData_Byte(0x11);
    LCD_WriteData_Byte(0x13);
    LCD_WriteData_Byte(0x2C);
    LCD_WriteData_Byte(0x3F);
    LCD_WriteData_Byte(0x44);
    LCD_WriteData_Byte(0x51);
    LCD_WriteData_Byte(0x2F);
    LCD_WriteData_Byte(0x1F);
    LCD_WriteData_Byte(0x1F);
    LCD_WriteData_Byte(0x20);
    LCD_WriteData_Byte(0x23);

    LCD_WriteReg(0x21);
    LCD_WriteReg(0x11);
    DEV_Delay_ms(120);
    LCD_WriteReg(0x29);
}

void LCD_SetCursor(UWORD x1, UWORD y1, UWORD x2, UWORD y2)
{
    // Waveshare official offsets for 135x240 display
    LCD_WriteReg(0x2A);
    LCD_WriteData_Word(x1 + 52);
    LCD_WriteData_Word(x2 + 52);
    LCD_WriteReg(0x2B);
    LCD_WriteData_Word(y1 + 40);
    LCD_WriteData_Word(y2 + 40);

    LCD_WriteReg(0x2C);
}

void LCD_Clear(UWORD Color)
{
    LCD_SetCursor(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    for(UWORD i = 0; i < LCD_WIDTH; i++) {
        for(UWORD j = 0; j < LCD_HEIGHT; j++) {
            LCD_WriteData_Word(Color);
        }
    }
}

void LCD_DrawPoint(UWORD X, UWORD Y, UWORD Color)
{
    LCD_SetCursor(X, Y, X, Y);
    LCD_WriteData_Word(Color);
}

void LCD_SetBacklight(UWORD Value)
{
    analogWrite(DEV_BL_PIN, Value / 4); // Convert to 0-255 range
}