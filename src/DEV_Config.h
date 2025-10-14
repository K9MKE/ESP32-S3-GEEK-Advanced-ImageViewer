/*****************************************************************************
* | File        :   DEV_Config.h
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master 
*                and enhance portability
*----------------
* | This version:   V1.0
* | Date        :   2018-11-22
* | Info        :
******************************************************************************/
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include <stdint.h>
#include <stdio.h>
#include <SPI.h>
#include <Arduino.h>

#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * GPIO config
**/
#define DEV_RST_PIN 9
#define DEV_DC_PIN  8
#define DEV_CS_PIN  10
#define DEV_BL_PIN  7

/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) digitalWrite(_pin, _value == 0? LOW:HIGH)
#define DEV_Digital_Read(_pin) digitalRead(_pin)

/**
 * SPI
**/
#define DEV_SPI_WRITE(_dat)   SPI.transfer(_dat)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms)    delay(__xms)

/**
 * PWM_BL
**/
 #define  DEV_Set_BL(_Pin, _Value)  analogWrite(_Pin, _Value)

/*-----------------------------------------------------------------------------*/
void Config_Init();
void GPIO_Init();

#endif