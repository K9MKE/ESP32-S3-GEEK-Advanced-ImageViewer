# ESP32-S3 Geek LCD Display Integration Guide

## Table of Contents
1. [Overview](#overview)
2. [Hardware Specifications](#hardware-specifications)
3. [Reusable Components](#reusable-components)
4. [Quick Start Setup](#quick-start-setup)
5. [Core Functions Reference](#core-functions-reference)
6. [Image Conversion Pipeline](#image-conversion-pipeline)
7. [Example Projects](#example-projects)
8. [Troubleshooting](#troubleshooting)
9. [Advanced Customization](#advanced-customization)

---

## Overview

This guide provides everything you need to integrate LCD display functionality into your ESP32-S3 projects using the proven components from the LCD_BUTTON project. The code has been tested and verified to work perfectly with the Waveshare ESP32-S3 Geek module's built-in ST7789V LCD display (135×240 pixels).

### What This Guide Covers
- ✅ Plug-and-play display driver components
- ✅ Image conversion from PNG to RGB565 format
- ✅ Hardware pin configurations that work
- ✅ Complete function reference
- ✅ Example implementations
- ✅ Common troubleshooting solutions

---

## Hardware Specifications

### ESP32-S3 Geek Module LCD
- **Display Controller**: ST7789V
- **Resolution**: 135×240 pixels (W×H)
- **Color Format**: RGB565 (16-bit color)
- **Interface**: SPI
- **Orientation**: Portrait mode (135 wide, 240 tall)

### Verified Pin Configuration
```cpp
#define DEV_RST_PIN    9   // Reset pin
#define DEV_DC_PIN     8   // Data/Command pin  
#define DEV_CS_PIN     10  // Chip Select pin
#define DEV_BL_PIN     7   // Backlight control pin
```

### SPI Settings (Tested & Working)
```cpp
SPI.setDataMode(SPI_MODE3);
SPI.setBitOrder(MSBFIRST);
SPI.setClockDivider(SPI_CLOCK_DIV2);
```

---

## Reusable Components

### Core Files to Copy to New Projects

#### 1. Hardware Abstraction Layer
- **File**: `src/DEV_Config.h`
- **Purpose**: Pin definitions and hardware interface
- **Status**: Ready to use as-is

#### 2. LCD Display Driver
- **Files**: `src/LCD_Driver.h`, `src/LCD_Driver.cpp`
- **Purpose**: ST7789V controller commands and display management
- **Key Features**:
  - Proper display window offsets (x+52, y+40)
  - Full-screen rendering without static borders
  - Backlight control
  - Color filling and pixel setting

#### 3. Graphics Library
- **Files**: `src/GUI_Paint.h`, `src/GUI_Paint.cpp`
- **Purpose**: Higher-level drawing functions
- **Key Features**:
  - Image rendering from PROGMEM
  - Pixel-level drawing
  - RGB565 color handling

#### 4. Image Conversion Script
- **File**: `scripts/convert_images.py`
- **Purpose**: Convert PNG files to C header arrays
- **Features**:
  - Automatic 90° rotation for correct orientation
  - RGB565 format conversion
  - PROGMEM-compatible output

---

## Quick Start Setup

### Step 1: Copy Required Files
Copy these files to your new project's `src/` directory:
```
src/DEV_Config.h
src/LCD_Driver.h
src/LCD_Driver.cpp
src/GUI_Paint.h
src/GUI_Paint.cpp
```

### Step 2: Copy Image Conversion Script
Copy to your new project root:
```
scripts/convert_images.py
```

### Step 3: Install Python Dependencies
```bash
python -m pip install Pillow
```

### Step 4: Basic Integration Code
```cpp
#include <Arduino.h>
#include <SPI.h>
#include "DEV_Config.h"
#include "LCD_Driver.h"
#include "GUI_Paint.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize hardware and display
  Config_Init();
  LCD_Init();
  LCD_SetBacklight(1000);  // Max brightness
  LCD_Clear(BLACK);
  
  // Initialize paint system
  Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 0, BLACK);
  
  Serial.println("Display ready!");
}

void loop() {
  // Your application code here
}
```

---

## Core Functions Reference

### Display Control Functions

#### `Config_Init()`
- **Purpose**: Initialize hardware pins and SPI
- **Call**: Once in setup()
- **Parameters**: None

#### `LCD_Init()`
- **Purpose**: Initialize ST7789V display controller
- **Call**: Once after Config_Init()
- **Parameters**: None

#### `LCD_SetBacklight(brightness)`
- **Purpose**: Control display brightness
- **Parameters**: 
  - `brightness`: 0-1000 (0=off, 1000=max)
- **Example**: `LCD_SetBacklight(500);`

#### `LCD_Clear(color)`
- **Purpose**: Fill entire screen with solid color
- **Parameters**:
  - `color`: RGB565 color value
- **Example**: `LCD_Clear(RED);`

### Drawing Functions

#### `Paint_NewImage(width, height, rotate, color)`
- **Purpose**: Initialize paint canvas
- **Parameters**:
  - `width`: Canvas width (use LCD_WIDTH)
  - `height`: Canvas height (use LCD_HEIGHT)
  - `rotate`: Rotation angle (usually 0)
  - `color`: Background color
- **Example**: `Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 0, BLACK);`

#### `Paint_SetPixel(x, y, color)`
- **Purpose**: Draw single pixel
- **Parameters**:
  - `x`: X coordinate (0-134)
  - `y`: Y coordinate (0-239)
  - `color`: RGB565 color
- **Example**: `Paint_SetPixel(67, 120, WHITE);`

#### `Paint_DrawImage(imageData, x, y, width, height)`
- **Purpose**: Draw RGB565 image from PROGMEM
- **Parameters**:
  - `imageData`: Pointer to image array
  - `x, y`: Top-left corner position
  - `width, height`: Image dimensions
- **Example**: `Paint_DrawImage(myImage, 0, 0, 135, 240);`

### Available Colors
```cpp
BLACK     = 0x0000
WHITE     = 0xFFFF
RED       = 0xF800
GREEN     = 0x07E0
BLUE      = 0x001F
YELLOW    = 0xFFE0
CYAN      = 0x7FFF
MAGENTA   = 0xF81F
```

---

## Image Conversion Pipeline

### Converting PNG Images to Display Format

#### Step 1: Prepare Your Images
- Place PNG files in your project root directory
- Any size/format PNG will work (script handles resizing)

#### Step 2: Run Conversion Script
```bash
python scripts/convert_images.py
```

#### Step 3: Include Generated Headers
```cpp
#include "pic1_data.h"  // Auto-generated
#include "pic2_data.h"  // Auto-generated

// In your code:
Paint_DrawImage(pic1_data, 0, 0, pic1_data_width, pic1_data_height);
```

### Script Features
- ✅ **Automatic rotation**: 90° clockwise for correct orientation
- ✅ **Perfect sizing**: Resizes to 135×240 pixels
- ✅ **RGB565 conversion**: 16-bit color format
- ✅ **PROGMEM storage**: Efficient flash memory usage
- ✅ **Multiple images**: Processes all PNG files found

### Manual Image Addition
To add new images to existing projects:
1. Copy PNG file to project root
2. Update `convert_images.py` to include new file
3. Run conversion script
4. Include new header file in your code

---

## Example Projects

### 1. Simple Image Slideshow
```cpp
#include "image1_data.h"
#include "image2_data.h"
#include "image3_data.h"

const uint16_t* images[] = {image1_data, image2_data, image3_data};
const uint16_t widths[] = {image1_data_width, image2_data_width, image3_data_width};
const uint16_t heights[] = {image1_data_height, image2_data_height, image3_data_height};
int currentImage = 0;

void loop() {
  Paint_DrawImage(images[currentImage], 0, 0, widths[currentImage], heights[currentImage]);
  currentImage = (currentImage + 1) % 3;
  delay(2000);
}
```

### 2. Sensor Display Dashboard
```cpp
void displaySensorData(float temperature, float humidity) {
  LCD_Clear(BLACK);
  
  // Draw temperature bar
  int tempHeight = (int)(temperature * 2);
  for(int y = 0; y < tempHeight; y++) {
    for(int x = 10; x < 30; x++) {
      Paint_SetPixel(x, 220-y, RED);
    }
  }
  
  // Draw humidity bar  
  int humHeight = (int)(humidity * 2);
  for(int y = 0; y < humHeight; y++) {
    for(int x = 50; x < 70; x++) {
      Paint_SetPixel(x, 220-y, BLUE);
    }
  }
}
```

### 3. Button-Controlled Menu
```cpp
#define BUTTON_PIN 0
int menuSelection = 0;
const char* menuItems[] = {"Settings", "Display", "Network", "About"};

void loop() {
  if(digitalRead(BUTTON_PIN) == LOW) {
    menuSelection = (menuSelection + 1) % 4;
    updateMenu();
    delay(300);
  }
}

void updateMenu() {
  LCD_Clear(BLACK);
  // Draw menu items with current selection highlighted
  for(int i = 0; i < 4; i++) {
    uint16_t color = (i == menuSelection) ? WHITE : GRAY;
    // Draw text at y position i*60
  }
}
```

---

## Troubleshooting

### Common Issues and Solutions

#### Problem: Display shows static or corrupted images
**Solution**: Check image dimensions and offsets
```cpp
// Verify display offsets in LCD_SetCursor():
x1 += 52;  // X offset
y1 += 40;  // Y offset
```

#### Problem: Images appear rotated wrong
**Solution**: Modify rotation in conversion script
```python
# In convert_images.py, adjust rotation:
img = img.rotate(-90, expand=True)  # 90° clockwise
# or
img = img.rotate(90, expand=True)   # 90° counter-clockwise
```

#### Problem: Display is blank/black
**Solution**: Check pin connections and initialization
```cpp
// Verify pin definitions match your hardware
#define DEV_RST_PIN    9
#define DEV_DC_PIN     8  
#define DEV_CS_PIN     10
#define DEV_BL_PIN     7

// Ensure proper initialization order
Config_Init();
LCD_Init();
LCD_SetBacklight(1000);
```

#### Problem: Colors are wrong
**Solution**: Verify RGB565 color format
```cpp
// Test with known colors:
LCD_Clear(RED);    // Should be red
LCD_Clear(GREEN);  // Should be green
LCD_Clear(BLUE);   // Should be blue
```

#### Problem: Compilation errors with image data
**Solution**: Check header file inclusion
```cpp
// Make sure to include:
#include <pgmspace.h>
#include "your_image_data.h"

// And use PROGMEM correctly:
const uint16_t myImage[] PROGMEM = { /* data */ };
```

### Hardware Debugging Steps

1. **Verify Power**: Check 3.3V supply to display
2. **Check Connections**: Verify SPI pins match code definitions
3. **Test SPI**: Use oscilloscope to verify SPI signals
4. **Backlight Test**: Try different brightness values
5. **Reset Sequence**: Ensure proper reset timing

---

## Advanced Customization

### Modifying Display Offsets
Different ST7789V panels may require different offsets:
```cpp
// In LCD_Driver.cpp, LCD_SetCursor() function:
void LCD_SetCursor(UWORD x1, UWORD y1, UWORD x2, UWORD y2) {
    LCD_WriteReg(0x2A);
    LCD_WriteData_Word(x1 + YOUR_X_OFFSET);  // Adjust X offset
    LCD_WriteData_Word(x2 + YOUR_X_OFFSET);
    LCD_WriteReg(0x2B);
    LCD_WriteData_Word(y1 + YOUR_Y_OFFSET);  // Adjust Y offset
    LCD_WriteData_Word(y2 + YOUR_Y_OFFSET);
    LCD_WriteReg(0x2C);
}
```

### Adding New Graphics Functions
Example: Draw a rectangle
```cpp
void Paint_DrawRectangle(UWORD x1, UWORD y1, UWORD x2, UWORD y2, UWORD color) {
    for(UWORD x = x1; x <= x2; x++) {
        Paint_SetPixel(x, y1, color);  // Top edge
        Paint_SetPixel(x, y2, color);  // Bottom edge
    }
    for(UWORD y = y1; y <= y2; y++) {
        Paint_SetPixel(x1, y, color);  // Left edge
        Paint_SetPixel(x2, y, color);  // Right edge
    }
}
```

### Custom Color Definitions
```cpp
// Define custom colors in RGB565 format
#define ORANGE    0xFD20
#define PURPLE    0x801F
#define PINK      0xF81F
#define LIME      0x07FF
```

### Performance Optimization
For faster drawing:
```cpp
// Use LCD_SetCursor() once, then write multiple pixels
LCD_SetCursor(x_start, y_start, x_end, y_end);
for(int i = 0; i < num_pixels; i++) {
    LCD_WriteData_Word(pixel_color);
}
```

---

## Conversion to PDF/Word

### For PDF:
1. Save this file as `ESP32_LCD_Guide.md`
2. Use online converters like:
   - https://www.markdowntopdf.com/
   - https://gitiles.com/markdown-to-pdf
   - Or install `pandoc`: `pandoc ESP32_LCD_Guide.md -o ESP32_LCD_Guide.pdf`

### For Word:
1. Use online converters:
   - https://word-to-markdown.herokuapp.com/ (reverse direction)
   - https://pandoc.org/ with: `pandoc ESP32_LCD_Guide.md -o ESP32_LCD_Guide.docx`
2. Or copy-paste into Word and format manually

### For Better Formatting:
Add these CSS styles when converting:
```css
body { font-family: Arial, sans-serif; line-height: 1.6; margin: 40px; }
code { background-color: #f4f4f4; padding: 2px 4px; border-radius: 3px; }
pre { background-color: #f4f4f4; padding: 10px; border-radius: 5px; overflow-x: auto; }
h1, h2, h3 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 5px; }
```

---

**Document Version**: 1.0  
**Last Updated**: October 14, 2025  
**Project**: ESP32-S3 Geek LCD_BUTTON  
**Hardware**: Waveshare ESP32-S3 Geek Module  

---

*This guide is based on proven, working code from the LCD_BUTTON project. All functions and configurations have been tested and verified on actual hardware.*