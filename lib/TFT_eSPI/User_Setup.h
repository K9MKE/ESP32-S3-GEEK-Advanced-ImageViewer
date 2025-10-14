// Configuration file for TFT_eSPI library
// ESP32-S3 Geek module pin definitions

// Define the display driver
#define ST7789_DRIVER

// Define the display size
#define TFT_WIDTH  240
#define TFT_HEIGHT 135

// Define the ESP32-S3 Geek pin connections
#define TFT_MOSI 11  // SDA pin
#define TFT_SCLK 12  // SCL pin
#define TFT_CS   10  // Chip select pin
#define TFT_DC   13  // Data Command pin
#define TFT_RST  14  // Reset pin

// Define backlight pin
#define TFT_BL   7   // Backlight control pin

// Define SPI frequency
#define SPI_FREQUENCY  40000000

// Font settings
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel high font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel high font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel high font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Define smooth fonts
#define SMOOTH_FONT