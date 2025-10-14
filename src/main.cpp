/*
 * ESP32-S3 Geek Module LCD_BUTTON Program
 * Display pic1.png and pic2.png images with button control
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <JPEGDEC.h>
#include "DEV_Config.h"
#include "LCD_Driver.h"
#include "GUI_Paint.h"
#include "image.h"

#define BUTTON_PIN     0   // BOOT button
#define LED_PIN        2   // Built-in LED  

// SD Card SPI pins for ESP32-S3 Geek
#define SDCARD_SCK     36  // SD card clock pin
#define SDCARD_MISO    37  // SD card data out pin  
#define SDCARD_MOSI    35  // SD card data in pin
#define SDCARD_SS_PIN  34  // SD card CS pin

// Custom SPI instance for SD card
SPIClass sdSPI(HSPI);

// Global variables
bool sdCardInitialized = false;
bool slideshowMode = false;
unsigned long slideshowTimer = 0;

// Slideshow speed control (intervals in milliseconds) - non-linear progression
const unsigned long SLIDESHOW_SPEEDS[] = {500, 1000, 2000, 3000, 5000, 10000, 15000, 30000}; // 0.5s to 30s
const int NUM_SPEEDS = 8;
int currentSpeedIndex = 4; // Start with 3 seconds (middle of range)
unsigned long currentSlideshowInterval = SLIDESHOW_SPEEDS[currentSpeedIndex];

const unsigned long BUTTON_HOLD_TIME = 2000; // 2 seconds to toggle mode (reduced from 3s)
const unsigned long DOUBLE_CLICK_TIME = 1200; // Increased to 1200ms for easier double-click
const unsigned long MODE_DISPLAY_TIME = 2000; // Show mode graphics for 2 seconds
const unsigned long SD_CHECK_INTERVAL = 3000; // Check for SD card every 3 seconds
const unsigned long SPEED_INDICATOR_TIME = 800; // Reduced to 800ms for faster UI

// Button state tracking
unsigned long buttonPressTime = 0;
unsigned long lastClickTime = 0;
bool buttonPressed = false;
bool holdDetected = false;
bool waitingForDoubleClick = false;

// Mode display tracking
bool showingModeGraphic = false;
unsigned long modeGraphicTimer = 0;

// Speed indicator overlay tracking
bool showingSpeedIndicator = false;
unsigned long speedIndicatorTimer = 0;
bool speedLimitBlink = false;
unsigned long speedLimitBlinkTimer = 0;
int speedLimitBlinkCount = 0;
const unsigned long SPEED_LIMIT_BLINK_TIME = 300; // 300ms per blink
const int SPEED_LIMIT_BLINK_TOTAL = 2; // Blink twice
bool speedIncreased = false; // true if speed was increased, false if decreased

// SD card monitoring
unsigned long lastSDCheck = 0;
bool lastSDCardState = false;

// Image management for dynamic loading
struct ImageInfo {
  String fileName;
  bool isEmbedded;
  const uint16_t* embeddedData;
  uint16_t* sdData;
  uint16_t width;
  uint16_t height;
};

const int MAX_TOTAL_IMAGES = 10;
ImageInfo imageList[MAX_TOTAL_IMAGES];
int totalImages = 0;
int currentImageIndex = 0;  

// Function declarations
void drawString(int x, int y, const char* str, uint16_t color);
void drawStringRotated(int x, int y, const char* str, uint16_t color);

void GPIO_Init() {
  pinMode(DEV_CS_PIN, OUTPUT);
  pinMode(DEV_RST_PIN, OUTPUT);
  pinMode(DEV_DC_PIN, OUTPUT);
  pinMode(DEV_BL_PIN, OUTPUT);
  analogWrite(DEV_BL_PIN, 140); // Set backlight
}

// Status image display functions
void displayStatusImage(const uint16_t* imageData, uint16_t width, uint16_t height) {
  LCD_Clear(BLACK);
  Paint_DrawImage(imageData, 0, 0, width, height);
}

void showNoSDCardStatus() {
  Serial.println("📱 Displaying: No SD Card");
  displayStatusImage(no_sd_card_data, no_sd_card_data_width, no_sd_card_data_height);
}

void showNoImagesFoundStatus() {
  Serial.println("📱 Displaying: No Images Found");
  displayStatusImage(no_images_found_data, no_images_found_data_width, no_images_found_data_height);
}

void showScanningStatus() {
  Serial.println("📱 Displaying: Scanning...");
  displayStatusImage(scanning_data, scanning_data_width, scanning_data_height);
}

void showManualModeStatus() {
  Serial.println("📱 Displaying: Manual Mode");
  displayStatusImage(manual_mode_data, manual_mode_data_width, manual_mode_data_height);
}

void showSlideshowModeStatus() {
  Serial.println("📱 Displaying: Slideshow Mode");
  displayStatusImage(slideshow_mode_data, slideshow_mode_data_width, slideshow_mode_data_height);
}

// Simple 5x7 pixel font patterns for digits and letters (normal upright orientation)
void drawChar(int x, int y, char c, uint16_t color) {
  // 5x7 pixel patterns for characters (standard upright orientation)
  uint8_t patterns[12][7] = {
    // '0'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // '1' 
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
    // '2'
    {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111},
    // '3'
    {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110},
    // '4'
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},
    // '5'
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},
    // '6'
    {0b01110, 0b10000, 0b11110, 0b10001, 0b10001, 0b10001, 0b01110},
    // '7'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},
    // '8'
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},
    // '9'
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110},
    // 's'
    {0b00000, 0b01111, 0b10000, 0b01110, 0b00001, 0b11110, 0b00000},
    // 'm'
    {0b00000, 0b11010, 0b10101, 0b10101, 0b10101, 0b10101, 0b00000}
  };
  
  int patternIndex = -1;
  if (c >= '0' && c <= '9') {
    patternIndex = c - '0';
  } else if (c == 's') {
    patternIndex = 10;
  } else if (c == 'm') {
    patternIndex = 11;
  }
  
  if (patternIndex >= 0) {
    for (int row = 0; row < 7; row++) {
      uint8_t pattern = patterns[patternIndex][row];
      for (int col = 0; col < 5; col++) {
        if (pattern & (1 << (4 - col))) {
          Paint_SetPixel(x + col, y + row, color);
        }
      }
    }
  }
}

// Rotated character drawing (90 degrees clockwise)
void drawCharRotated(int x, int y, char c, uint16_t color) {
  // Same patterns but drawn rotated 90 degrees clockwise
  uint8_t patterns[12][7] = {
    // '0'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // '1' 
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
    // '2'
    {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111},
    // '3'
    {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110},
    // '4'
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},
    // '5'
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},
    // '6'
    {0b01110, 0b10000, 0b11110, 0b10001, 0b10001, 0b10001, 0b01110},
    // '7'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},
    // '8'
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},
    // '9'
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110},
    // 's'
    {0b00000, 0b01111, 0b10000, 0b01110, 0b00001, 0b11110, 0b00000},
    // 'm'
    {0b00000, 0b11010, 0b10101, 0b10101, 0b10101, 0b10101, 0b00000}
  };
  
  int patternIndex = -1;
  if (c >= '0' && c <= '9') {
    patternIndex = c - '0';
  } else if (c == 's') {
    patternIndex = 10;
  } else if (c == 'm') {
    patternIndex = 11;
  }
  
  if (patternIndex >= 0) {
    // Draw rotated 90 degrees clockwise, fix vertical mirroring
    for (int row = 0; row < 7; row++) {
      uint8_t pattern = patterns[patternIndex][row];
      for (int col = 0; col < 5; col++) {
        if (pattern & (1 << (4 - col))) { // Back to original bit order
          // Rotate 90° clockwise and fix vertical mirror: new_x = (6-row), new_y = col
          Paint_SetPixel(x + (6 - row), y + col, color);
        }
      }
    }
  }
}

void drawString(int x, int y, const char* str, uint16_t color) {
  int pos = 0;
  while (*str) {
    if (*str == '.') {
      // Draw dot
      Paint_SetPixel(x + pos * 6 + 2, y + 6, color);
      pos++;
    } else {
      drawChar(x + pos * 6, y, *str, color);
      pos++;
    }
    str++;
  }
}

void drawStringRotated(int x, int y, const char* str, uint16_t color) {
  int pos = 0;
  while (*str) {
    if (*str == '.') {
      // Draw dot (rotated)
      Paint_SetPixel(x + 3, y + pos * 8 + 3, color);
      pos++;
    } else {
      drawCharRotated(x, y + pos * 8, *str, color);
      pos++;
    }
    str++;
  }
}

void drawSpeedIndicator(bool increased) {
  // Speed indicator overlay in upper left corner (rotated 90° clockwise)
  int indicator_x = LCD_WIDTH - 25; // Right side for landscape upper left
  int indicator_y = 5; // Top edge
  int width = 20; // Swapped width/height for 90° rotation
  int height = 50;
  
  // FlipperZero orange color (RGB565 format)
  uint16_t orange = 0xFD20; // Orange color in RGB565
  uint16_t black = 0x0000;
  
  // Clear the indicator area with black background
  for (int y = indicator_y; y < indicator_y + height; y++) {
    for (int x = indicator_x; x < indicator_x + width; x++) {
      Paint_SetPixel(x, y, black);
    }
  }
  
  // Draw border efficiently
  for (int i = 0; i < width; i++) {
    Paint_SetPixel(indicator_x + i, indicator_y, orange);
    Paint_SetPixel(indicator_x + i, indicator_y + height - 1, orange);
  }
  for (int i = 0; i < height; i++) {
    Paint_SetPixel(indicator_x, indicator_y + i, orange);
    Paint_SetPixel(indicator_x + width - 1, indicator_y + i, orange);
  }
  
  // Format speed text
  float speedInSeconds = currentSlideshowInterval / 1000.0;
  char speedText[10];
  
  if (speedInSeconds < 1.0) {
    // Show as milliseconds for sub-second speeds
    sprintf(speedText, "%dms", currentSlideshowInterval);
  } else if (speedInSeconds == (int)speedInSeconds) {
    // Show as whole seconds (no decimal)
    sprintf(speedText, "%ds", (int)speedInSeconds);
  } else {
    // Show with one decimal place
    sprintf(speedText, "%.1fs", speedInSeconds);
  }
  
  // Calculate text position for vertical text layout (rotated 90° clockwise)
  int textLen = strlen(speedText);
  int textHeight = textLen * 8; // 8 pixels per char vertically
  int text_x = indicator_x + 7; // Horizontally centered in 20px width
  int text_y = indicator_y + (height - textHeight) / 2; // Vertically centered
  
  // Draw the speed text (rotated 90° clockwise)
  drawStringRotated(text_x, text_y, speedText, orange);
}

void Config_Init() {
  GPIO_Init();
  
  // SPI setup exactly like Waveshare code
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
}

void initializeEmbeddedImages() {
#ifdef ENABLE_EMBEDDED_IMAGES
  Serial.println("📸 Initializing embedded images...");
  
  // Example embedded image setup (uncomment and modify as needed):
  /*
  // Add pic1_data
  imageList[totalImages].fileName = "pic1.png";
  imageList[totalImages].isEmbedded = true;
  imageList[totalImages].embeddedData = pic1_data;
  imageList[totalImages].sdData = nullptr;
  imageList[totalImages].width = pic1_data_width;
  imageList[totalImages].height = pic1_data_height;
  totalImages++;
  
  // Add pic2_data
  imageList[totalImages].fileName = "pic2.png";
  imageList[totalImages].isEmbedded = true;
  imageList[totalImages].embeddedData = pic2_data;
  imageList[totalImages].sdData = nullptr;
  imageList[totalImages].width = pic2_data_width;
  imageList[totalImages].height = pic2_data_height;
  totalImages++;
  */
  
  Serial.println("✅ " + String(totalImages) + " embedded images initialized");
#else
  Serial.println("📸 Embedded images disabled - using SD card images only");
  Serial.println("💡 To enable embedded images, uncomment #define ENABLE_EMBEDDED_IMAGES in image.h");
#endif
}

// Global JPEG decoder
JPEGDEC jpeg;

// Callback function for JPEG decoder to draw pixels
int JPEGDraw(JPEGDRAW *pDraw) {
  // Get the target image buffer
  ImageInfo* currentImg = (ImageInfo*)pDraw->pUser;
  
  if (currentImg->sdData == nullptr) return 0;
  
  // Get the pixel data from decoder
  uint16_t* pixels = pDraw->pPixels;
  int x = pDraw->x;
  int y = pDraw->y;
  int w = pDraw->iWidth;
  int h = pDraw->iHeight;
  
  // Debug info for first few calls
  static int callCount = 0;
  if (callCount < 3) {
    Serial.println("🎨 JPEGDraw: x=" + String(x) + " y=" + String(y) + " w=" + String(w) + " h=" + String(h));
    callCount++;
  }
  
  // Bounds checking
  if (x >= currentImg->width || y >= currentImg->height) return 1;
  if (x + w > currentImg->width) w = currentImg->width - x;
  if (y + h > currentImg->height) h = currentImg->height - y;
  
  // Copy pixels to our buffer
  for (int row = 0; row < h; row++) {
    if ((y + row) >= currentImg->height) break;
    
    int srcOffset = row * pDraw->iWidth; // Source line offset
    int dstOffset = (y + row) * currentImg->width + x; // Destination offset
    
    // Copy one line at a time, with bounds checking
    int copyWidth = min(w, currentImg->width - x);
    if (copyWidth > 0) {
      memcpy(&currentImg->sdData[dstOffset], &pixels[srcOffset], copyWidth * sizeof(uint16_t));
    }
  }
  
  return 1; // Return 1 to continue decoding
}

uint16_t* loadJPEGFromSD(const String& fileName, uint16_t* width, uint16_t* height) {
  Serial.println("📖 Loading JPEG: " + fileName);
  
  File jpegFile = SD.open("/" + fileName);
  if (!jpegFile) {
    Serial.println("❌ Failed to open: " + fileName);
    return nullptr;
  }
  
  // Read entire file into memory
  size_t fileSize = jpegFile.size();
  uint8_t* jpegData = (uint8_t*)malloc(fileSize);
  if (jpegData == nullptr) {
    Serial.println("❌ Failed to allocate memory for JPEG data");
    jpegFile.close();
    return nullptr;
  }
  
  jpegFile.read(jpegData, fileSize);
  jpegFile.close();
  
  // Initialize JPEG decoder
  if (!jpeg.openRAM(jpegData, fileSize, JPEGDraw)) {
    Serial.println("❌ Failed to open JPEG");
    free(jpegData);
    return nullptr;
  }
  
  int imgWidth = jpeg.getWidth();
  int imgHeight = jpeg.getHeight();
  
  Serial.println("📏 Original size: " + String(imgWidth) + "x" + String(imgHeight));
  
  // Display dimensions
  int displayWidth = 135;
  int displayHeight = 240;
  
  // Allocate buffer for final image (full display size)
  uint16_t* imageData = (uint16_t*)malloc(displayWidth * displayHeight * sizeof(uint16_t));
  if (imageData == nullptr) {
    Serial.println("❌ Failed to allocate image buffer");
    jpeg.close();
    free(jpegData);
    return nullptr;
  }
  
  // Fill with black for letterbox background
  memset(imageData, 0, displayWidth * displayHeight * sizeof(uint16_t));
  
  // Calculate the best scale that fits within display bounds
  // Since we're rotating 90° clockwise, we need to swap width/height for scaling calculation
  float scaleX = (float)displayWidth / imgHeight;  // Note: swapped because of rotation
  float scaleY = (float)displayHeight / imgWidth;   // Note: swapped because of rotation
  float scale = min(scaleX, scaleY);
  
  // Calculate final scaled dimensions (after rotation)
  int finalWidth = (int)(imgHeight * scale);   // Note: swapped because of rotation
  int finalHeight = (int)(imgWidth * scale);   // Note: swapped because of rotation
  
  // Calculate centering offsets
  int offsetX = (displayWidth - finalWidth) / 2;
  int offsetY = (displayHeight - finalHeight) / 2;
  
  Serial.println("📐 Target size (after rotation): " + String(finalWidth) + "x" + String(finalHeight));
  Serial.println("📍 Offset: (" + String(offsetX) + ", " + String(offsetY) + ")");
  Serial.println("🔍 Scale factor: " + String(scale, 3));
  
  // Use built-in JPEG scaling if possible
  int scaleFlag = 0;
  if (scale <= 0.125) scaleFlag = JPEG_SCALE_EIGHTH;
  else if (scale <= 0.25) scaleFlag = JPEG_SCALE_QUARTER;
  else if (scale <= 0.5) scaleFlag = JPEG_SCALE_HALF;
  
  if (scaleFlag != 0) {
    Serial.println("🎯 Using hardware scale: " + String(scaleFlag));
  }
  
  // Create a temporary larger buffer for the decoded JPEG
  // Then we'll scale it down to the final size
  int tempWidth = imgWidth;
  int tempHeight = imgHeight;
  
  if (scaleFlag != 0) {
    // Adjust temp size based on hardware scaling
    switch(scaleFlag) {
      case JPEG_SCALE_EIGHTH: tempWidth /= 8; tempHeight /= 8; break;
      case JPEG_SCALE_QUARTER: tempWidth /= 4; tempHeight /= 4; break;
      case JPEG_SCALE_HALF: tempWidth /= 2; tempHeight /= 2; break;
    }
  }
  
  uint16_t* tempBuffer = (uint16_t*)malloc(tempWidth * tempHeight * sizeof(uint16_t));
  if (tempBuffer == nullptr) {
    Serial.println("❌ Failed to allocate temp buffer");
    free(imageData);
    jpeg.close();
    free(jpegData);
    return nullptr;
  }
  
  // Create temp ImageInfo for decoding
  ImageInfo tempImg;
  tempImg.sdData = tempBuffer;
  tempImg.width = tempWidth;
  tempImg.height = tempHeight;
  jpeg.setUserPointer(&tempImg);
  
  // Decode JPEG to temp buffer
  jpeg.setPixelType(RGB565_LITTLE_ENDIAN);
  
  bool success = jpeg.decode(0, 0, scaleFlag);
  
  if (success) {
    Serial.println("✅ JPEG decoded to temp buffer");
    
    // Now manually scale/copy from temp buffer to final buffer with 270° clockwise rotation (90° + 180°)
    // For 270° clockwise rotation (or 90° counter-clockwise): new_x = old_height - 1 - old_y, new_y = old_x
    
    for (int srcY = 0; srcY < tempHeight; srcY++) {
      for (int srcX = 0; srcX < tempWidth; srcX++) {
        // Source pixel index
        int srcIndex = srcY * tempWidth + srcX;
        
        // Apply 270° clockwise rotation (which is 90° counter-clockwise)
        int rotatedX = tempHeight - 1 - srcY;  // new_x = old_height - 1 - old_y
        int rotatedY = srcX;                   // new_y = old_x
        
        // Scale the rotated coordinates to final size
        int finalX = (rotatedX * finalWidth) / tempHeight;
        int finalY = (rotatedY * finalHeight) / tempWidth;
        
        // Apply centering offset
        finalX += offsetX;
        finalY += offsetY;
        
        // Bounds check and copy pixel
        if (finalX >= 0 && finalX < displayWidth && finalY >= 0 && finalY < displayHeight) {
          int dstIndex = finalY * displayWidth + finalX;
          if (dstIndex >= 0 && dstIndex < displayWidth * displayHeight) {
            imageData[dstIndex] = tempBuffer[srcIndex];
          }
        }
      }
    }
    
    Serial.println("✅ Scaled, letterboxed, and rotated 270° clockwise (corrected orientation)");
    *width = displayWidth;
    *height = displayHeight;
  } else {
    Serial.println("❌ JPEG decode failed");
    free(imageData);
    imageData = nullptr;
  }
  
  free(tempBuffer);
  jpeg.close();
  free(jpegData);
  
  return imageData;
}

void loadSDCardImages() {
  if (!sdCardInitialized || totalImages >= MAX_TOTAL_IMAGES) return;
  
  Serial.println("🔍 Loading images from SD card...");
  File root = SD.open("/");
  if (!root) {
    Serial.println("❌ Failed to open SD root directory");
    return;
  }
  
  int sdImagesLoaded = 0;
  File file = root.openNextFile();
  while (file && totalImages < MAX_TOTAL_IMAGES) {
    if (!file.isDirectory()) {
      String fileName = file.name();
      String lowerName = fileName;
      lowerName.toLowerCase();
      
      // Check for supported image formats
      if (lowerName.endsWith(".jpg") || lowerName.endsWith(".jpeg")) {
        Serial.println("🖼️  Loading JPEG: " + fileName);
        
        uint16_t width, height;
        uint16_t* imageData = loadJPEGFromSD(fileName, &width, &height);
        
        if (imageData != nullptr) {
          imageList[totalImages].fileName = fileName;
          imageList[totalImages].isEmbedded = false;
          imageList[totalImages].embeddedData = nullptr;
          imageList[totalImages].sdData = imageData;
          imageList[totalImages].width = width;
          imageList[totalImages].height = height;
          totalImages++;
          sdImagesLoaded++;
          Serial.println("✅ Loaded JPEG: " + fileName);
        }
      } else if (lowerName.endsWith(".png") || lowerName.endsWith(".bmp")) {
        Serial.println("⚠️  PNG/BMP not yet supported: " + fileName);
        // TODO: Add PNG/BMP support later if needed
      }
    }
    file = root.openNextFile();
  }
  
  root.close();
  Serial.println("✅ SD card scan complete - loaded " + String(sdImagesLoaded) + " JPEG images");
}

void displayCurrentImage() {
  if (totalImages == 0) return;
  
  ImageInfo& img = imageList[currentImageIndex];
  String modeText = slideshowMode ? 
    "(Slideshow " + String(currentSlideshowInterval/1000.0, 1) + "s)" : 
    "(Manual)";
  Serial.println("🖼️  Displaying (" + String(currentImageIndex + 1) + "/" + String(totalImages) + "): " + img.fileName + " " + modeText);
  
  if (img.isEmbedded) {
    Paint_DrawImage(img.embeddedData, 0, 0, img.width, img.height);
  } else {
    Paint_DrawImage(img.sdData, 0, 0, img.width, img.height);
  }
}

void nextImage() {
  if (totalImages > 0) {
    currentImageIndex = (currentImageIndex + 1) % totalImages;
    displayCurrentImage();
  }
}

void toggleSlideshowMode() {
  slideshowMode = !slideshowMode;
  slideshowTimer = millis();
  
  if (slideshowMode) {
    Serial.println("🎬 Slideshow mode ENABLED - Auto-advancing every " + String(currentSlideshowInterval/1000.0, 1) + " seconds");
    Serial.println("💡 Single press: Slower | Double press: Faster | Hold 2s: Manual mode");
    showSlideshowModeStatus();
  } else {
    Serial.println("⏸️  Manual mode ENABLED - Press button to advance");
    Serial.println("💡 Press: Next image | Hold 2s: Slideshow mode");
    showManualModeStatus();
  }
  
  // Set mode display timer (non-blocking)
  showingModeGraphic = true;
  modeGraphicTimer = millis();
}

void checkAndReloadSDCard() {
  // Try to reinitialize SD card
  bool currentSDState = SD.begin(SDCARD_SS_PIN, sdSPI, 80000000);
  
  if (currentSDState != lastSDCardState) {
    if (currentSDState) {
      Serial.println("🔄 SD Card detected! Reloading images...");
      showScanningStatus();
      delay(500);
      
      // Clear existing SD card images
      for (int i = 0; i < totalImages; i++) {
        if (!imageList[i].isEmbedded && imageList[i].sdData != nullptr) {
          free(imageList[i].sdData);
          imageList[i].sdData = nullptr;
        }
      }
      
      // Reset to only embedded images
      int embeddedCount = 0;
      for (int i = 0; i < totalImages; i++) {
        if (imageList[i].isEmbedded) {
          if (i != embeddedCount) {
            imageList[embeddedCount] = imageList[i];
          }
          embeddedCount++;
        }
      }
      totalImages = embeddedCount;
      
      // Reload SD card images
      sdCardInitialized = true;
      loadSDCardImages();
      currentImageIndex = 0;
      
      Serial.println("✅ SD Card reloaded! Total images: " + String(totalImages));
      
      if (totalImages > 0) {
        displayCurrentImage();
      } else {
        showNoImagesFoundStatus();
      }
    } else {
      Serial.println("⚠️  SD Card removed!");
      sdCardInitialized = false;
      
      // Keep only embedded images
      int embeddedCount = 0;
      for (int i = 0; i < totalImages; i++) {
        if (imageList[i].isEmbedded) {
          if (i != embeddedCount) {
            imageList[embeddedCount] = imageList[i];
          }
          embeddedCount++;
        } else if (imageList[i].sdData != nullptr) {
          free(imageList[i].sdData);
          imageList[i].sdData = nullptr;
        }
      }
      totalImages = embeddedCount;
      currentImageIndex = 0;
      
      if (totalImages > 0) {
        displayCurrentImage();
      } else {
        showNoSDCardStatus();
      }
    }
    lastSDCardState = currentSDState;
  }
}

void adjustSlideshowSpeed(bool increase) {
  bool hitLimit = false;
  
  if (increase && currentSpeedIndex > 0) {
    currentSpeedIndex--; // Lower index = faster (shorter interval)
    currentSlideshowInterval = SLIDESHOW_SPEEDS[currentSpeedIndex];
    Serial.println("⚡ Slideshow speed INCREASED to " + String(currentSlideshowInterval/1000.0, 2) + " seconds");
    speedIncreased = true;
  } else if (!increase && currentSpeedIndex < NUM_SPEEDS - 1) {
    currentSpeedIndex++; // Higher index = slower (longer interval)
    currentSlideshowInterval = SLIDESHOW_SPEEDS[currentSpeedIndex];
    Serial.println("🐌 Slideshow speed DECREASED to " + String(currentSlideshowInterval/1000.0, 2) + " seconds");
    speedIncreased = false;
  } else {
    Serial.println("⚠️  Speed limit reached!");
    hitLimit = true;
    speedIncreased = increase; // Show the direction that was attempted
  }
  
  // Show speed indicator overlay immediately
  if (totalImages > 0 && !showingModeGraphic) {
    showingSpeedIndicator = true;
    speedIndicatorTimer = millis();
    
    // If we hit a limit, set up blinking
    if (hitLimit) {
      speedLimitBlink = true;
      speedLimitBlinkTimer = millis();
      speedLimitBlinkCount = 0;
    } else {
      speedLimitBlink = false;
    }
    
    drawSpeedIndicator(speedIncreased);
  }
  
  // Reset timer with new interval (only if speed actually changed)
  if (!hitLimit) {
    slideshowTimer = millis();
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  
  Serial.println("===================================");
  Serial.println("ESP32-S3 Geek LCD_BUTTON Program");
  Serial.println("===================================");
  Serial.println("Cycling between pic1.png, pic2.png, and JPG image");
  Serial.println();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Flash LED to show starting
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
  
  Serial.println("Initializing hardware...");
  Config_Init();
  Serial.println("✅ Hardware initialized");
  
  Serial.println("Initializing LCD display...");
  LCD_Init();
  Serial.println("✅ LCD initialized successfully!");
  
  Serial.println("Setting backlight...");
  LCD_SetBacklight(1000);
  Serial.println("✅ Backlight set");
  
  Serial.println("Clearing screen and initializing paint...");
  LCD_Clear(BLACK);
  Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 0, BLACK);
  Serial.println("✅ Paint initialized");
  
  // Initialize embedded images first
  initializeEmbeddedImages();
  
  // Initialize SD card (required for image loading)
  Serial.println();
  Serial.println("Initializing SD card...");
  
  // Show scanning status
  showScanningStatus();
  delay(500); // Brief scanning display
  
  sdSPI.begin(SDCARD_SCK, SDCARD_MISO, SDCARD_MOSI, SDCARD_SS_PIN);
  if (SD.begin(SDCARD_SS_PIN, sdSPI, 80000000)) {
    sdCardInitialized = true;
    lastSDCardState = true;
    Serial.println("✅ SD Card initialized successfully!");
    
    // Load images from SD card
    loadSDCardImages();
    
    // List all available images
    Serial.println("📁 All available images:");
    for (int i = 0; i < totalImages; i++) {
      String type = imageList[i].isEmbedded ? "(embedded)" : "(SD card)";
      Serial.println("  " + String(i+1) + ". " + imageList[i].fileName + " " + type);
    }
  } else {
    Serial.println("❌ SD Card initialization failed!");
    sdCardInitialized = false;
    lastSDCardState = false;
    showNoSDCardStatus();
    delay(2000); // Show error longer
  }
  
  // Display first image immediately if available, otherwise show status
  if (totalImages > 0) {
    Serial.println("🖼️  Going directly to images...");
    displayCurrentImage(); // Skip directly to first image
    Serial.println("✅ Image display ready");
  } else {
    Serial.println("⚠️  No images found! Please add JPEG files to SD card root directory");
    showNoImagesFoundStatus();
  }
  
  Serial.println();
  Serial.println("🎉 FLIPPER-STYLE IMAGE VIEWER READY!");
  Serial.println("📸 Total images available: " + String(totalImages));
  if (sdCardInitialized) {
    Serial.println("💾 SD Card: Ready");
  } else {
    Serial.println("💾 SD Card: Not found - will auto-detect when inserted");
  }
  Serial.println("🔄 Auto SD card detection: Every 3 seconds");
  Serial.println("📋 Controls:");
  Serial.println("   Manual Mode:");
  Serial.println("     • Press: Next image");
  Serial.println("     • Hold 2s: Enable slideshow");
  Serial.println("   Slideshow Mode:");
  Serial.println("     • Single press: Slower (0.25s to 30s range)");
  Serial.println("     • Double press: Faster (with speed overlay)");
  Serial.println("     • Hold 2s: Manual mode");
  Serial.println("📱 FlipperZero-style status screens");
}

void loop() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  static bool lastButtonState = HIGH;
  
  // Blink LED every second to show we're alive
  if (millis() - lastBlink > 1000) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }
  
  // Handle mode graphic display timeout
  if (showingModeGraphic && (millis() - modeGraphicTimer > MODE_DISPLAY_TIME)) {
    showingModeGraphic = false;
    // Return to current image if we have any
    if (totalImages > 0) {
      displayCurrentImage();
    }
  }
  
  // Handle speed indicator timeout and blinking
  if (showingSpeedIndicator) {
    bool shouldHide = false;
    
    if (speedLimitBlink) {
      // Handle limit blinking - blink multiple times
      if (millis() - speedLimitBlinkTimer > SPEED_LIMIT_BLINK_TIME) {
        speedLimitBlinkCount++;
        speedLimitBlinkTimer = millis();
        
        if (speedLimitBlinkCount >= SPEED_LIMIT_BLINK_TOTAL) {
          speedLimitBlink = false;
          shouldHide = true;
        } else {
          // Redraw to create blink effect
          if (totalImages > 0 && !showingModeGraphic) {
            displayCurrentImage();
            // Small delay then redraw indicator
            delay(50);
            drawSpeedIndicator(speedIncreased);
          }
        }
      }
    } else if (millis() - speedIndicatorTimer > SPEED_INDICATOR_TIME) {
      // Normal timeout
      shouldHide = true;
    }
    
    if (shouldHide) {
      showingSpeedIndicator = false;
      speedLimitBlink = false;
      speedLimitBlinkCount = 0;
      // Redraw current image to clear the overlay
      if (totalImages > 0 && !showingModeGraphic) {
        displayCurrentImage();
      }
    }
  }
  
  // Check for SD card changes periodically
  if (millis() - lastSDCheck > SD_CHECK_INTERVAL) {
    checkAndReloadSDCard();
    lastSDCheck = millis();
  }
  
  // Handle slideshow mode auto-advance (only when not showing mode graphic and speed indicator finished drawing)
  if (slideshowMode && totalImages > 1 && !showingModeGraphic) {
    // Small delay after speed indicator starts to let it finish drawing
    bool speedIndicatorDrawing = showingSpeedIndicator && (millis() - speedIndicatorTimer < 50);
    
    if (!speedIndicatorDrawing && millis() - slideshowTimer > currentSlideshowInterval) {
      nextImage();
      slideshowTimer = millis();
      // Clear speed indicator when advancing to next image
      if (showingSpeedIndicator) {
        showingSpeedIndicator = false;
      }
    }
  }
  
  // Handle double-click timeout - MUST happen before button detection
  if (waitingForDoubleClick && (millis() - lastClickTime > DOUBLE_CLICK_TIME)) {
    waitingForDoubleClick = false;
    // Process the single click that was waiting
    if (slideshowMode) {
      adjustSlideshowSpeed(false); // Single click = slower
    }
  }
  
  // Advanced button handling
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  // Button press detection (falling edge)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    buttonPressed = true;
    buttonPressTime = millis();
    holdDetected = false;
  }
  
  // Button hold detection (while pressed)
  if (buttonPressed && currentButtonState == LOW) {
    if (!holdDetected && (millis() - buttonPressTime > BUTTON_HOLD_TIME)) {
      holdDetected = true;
      waitingForDoubleClick = false; // Cancel any pending actions
      toggleSlideshowMode();
      
      // Flash LED to indicate mode change
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
      }
    }
  }
  
  // Button release detection (rising edge)
  if (lastButtonState == LOW && currentButtonState == HIGH) {
    if (buttonPressed && !holdDetected) {
      unsigned long currentTime = millis();
      
      if (slideshowMode) {
        // In slideshow mode: handle single vs double click for speed control
        if (waitingForDoubleClick && (currentTime - lastClickTime < DOUBLE_CLICK_TIME)) {
          // Double-click detected - increase speed immediately
          waitingForDoubleClick = false;
          adjustSlideshowSpeed(true);
        } else {
          // First click - start waiting for potential double-click
          waitingForDoubleClick = true;
          lastClickTime = currentTime;
        }
      } else {
        // In manual mode: single click = next image (immediate)
        if (totalImages > 0) {
          nextImage();
        }
        waitingForDoubleClick = false;
      }
    }
    buttonPressed = false;
    holdDetected = false;
  }
  
  lastButtonState = currentButtonState;
  delay(10);
}