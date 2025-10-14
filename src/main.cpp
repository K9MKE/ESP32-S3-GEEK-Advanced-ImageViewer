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

void GPIO_Init() {
  pinMode(DEV_CS_PIN, OUTPUT);
  pinMode(DEV_RST_PIN, OUTPUT);
  pinMode(DEV_DC_PIN, OUTPUT);
  pinMode(DEV_BL_PIN, OUTPUT);
  analogWrite(DEV_BL_PIN, 140); // Set backlight
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
  Serial.println("🖼️  Displaying (" + String(currentImageIndex + 1) + "/" + String(totalImages) + "): " + img.fileName);
  
  if (img.isEmbedded) {
    Paint_DrawImage(img.embeddedData, 0, 0, img.width, img.height);
  } else {
    Paint_DrawImage(img.sdData, 0, 0, img.width, img.height);
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
  sdSPI.begin(SDCARD_SCK, SDCARD_MISO, SDCARD_MOSI, SDCARD_SS_PIN);
  if (SD.begin(SDCARD_SS_PIN, sdSPI, 80000000)) {
    sdCardInitialized = true;
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
  }
  
  // Display first image if any images are available
  if (totalImages > 0) {
    Serial.println("Displaying first image...");
    displayCurrentImage();
    Serial.println("✅ First image displayed");
  } else {
    Serial.println("⚠️  No images found! Please add JPEG files to SD card root directory");
    // Clear screen to show message - simple colored background
    LCD_Clear(RED);  // Red background to indicate no images
  }
  
  Serial.println();
  Serial.println("🎉 LCD_BUTTON PROGRAM READY!");
  Serial.println("📸 Total images available: " + String(totalImages));
  if (sdCardInitialized) {
    Serial.println("💾 SD Card: Ready");
  }
  Serial.println("Press BOOT button to cycle through all images!");
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
  
  // Check button for image switching (only if images are available)
  if (totalImages > 0) {
    bool buttonState = digitalRead(BUTTON_PIN);
    
    // Button pressed (falling edge)
    if (lastButtonState == HIGH && buttonState == LOW) {
      currentImageIndex = (currentImageIndex + 1) % totalImages;
      displayCurrentImage();
      delay(300); // Debounce delay
    }
    
    lastButtonState = buttonState;
  }
  delay(10);
}