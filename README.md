# ESP32-S3-GEEK Advanced Image Viewer

A sophisticated image slideshow system for the Waveshare ESP32-S3-GEEK module with professional UI design and advanced interactive controls.

## Hardware Requirements

- **Waveshare ESP32-S3 Geek Module** (240x135 ST7789V LCD display)
- **MicroSD card** (FAT32 formatted)
- **JPEG image files** on SD card root directory

## Features

### 🖼️ **Image Support**
- **SD Card JPEG**: Automatic loading of all JPEG/JPG files from SD card root
- **Automatic Scaling**: Images scaled to fit display while preserving aspect ratio
- **Letterboxing**: Black bars added when needed to maintain proportions
- **270° Rotation**: SD card images automatically rotated for correct orientation
- **Optional Embedded Images**: Add logos/icons to flash memory (see EMBEDDED_IMAGES_GUIDE.md)

### 🎮 **Controls**
- **Single Press**: Advance to next image / change slideshow speed
- **Double Press**: Toggle slideshow speed (8 levels: 0.5s to 30s)
- **Hold 2 seconds**: Toggle between Manual and Slideshow modes
- **LED Indicator (GPIO2)**: Shows system status and mode changes

### 🖥️ **Professional Interface**
- **Status Screens**: Clean orange-on-black UI graphics (design inspired by retro devices)
- **Mode Indicators**: Visual feedback for Manual vs Slideshow modes  
- **Speed Control**: 8-level slideshow timing (500ms to 30s)
- **Visual Feedback**: Speed limit blinking and real-time overlays
- **Hot-Swap Support**: Insert/remove SD card without restart

### 💾 **Storage**
- **SD Card**: External JPEG files (primary storage method)
- **Flash Memory**: Optional embedded images for logos/icons (disabled by default)
- **Memory Management**: Images loaded into RAM for instant switching

## Pin Configuration

```cpp
// Display (ST7789V)
#define DEV_CS_PIN     10   // TFT Chip Select
#define DEV_RST_PIN    14   // TFT Reset
#define DEV_DC_PIN     13   // TFT Data/Command
#define DEV_BL_PIN     2    // TFT Backlight
// SPI: MOSI=11, SCLK=12

// SD Card (Custom SPI)
#define SDCARD_SCK     36   // SD Clock
#define SDCARD_MISO    37   // SD Data Out
#define SDCARD_MOSI    35   // SD Data In  
#define SDCARD_SS_PIN  34   // SD Chip Select

// Controls
#define BUTTON_PIN     0    // BOOT button
#define LED_PIN        2    // Built-in LED
```

## How It Works

### 1. **Startup Sequence**
1. Initialize hardware (LCD, SPI, GPIO)
2. Check for embedded images (disabled by default)
3. Initialize SD card with custom SPI configuration
4. Scan SD card root for JPEG files
5. Load and process each JPEG file found
6. Display first image or "No Images Found" message

### 2. **JPEG Processing Pipeline**
```
SD JPEG File → RAM Buffer → JPEG Decode → Temp Buffer → 
Scale + Rotate + Letterbox → Final Display Buffer → LCD
```

**Steps:**
- Read entire JPEG file into memory
- Decode using JPEGDEC library
- Calculate optimal scale to fit 135x240 display
- Apply 270° clockwise rotation for correct orientation  
- Add letterbox bars (black) to center the image
- Copy to final RGB565 display buffer

### 3. **Image Management**
- **Array-based storage**: Up to 10 total images in `imageList[]`
- **SD Card focus**: Primarily loads JPEG files from SD card
- **Optional embedded**: Can add logos/icons to flash (see EMBEDDED_IMAGES_GUIDE.md)
- **Efficient switching**: All images pre-loaded for instant display
- **Memory cleanup**: Automatic management of dynamically allocated buffers

### 4. **User Interaction**
- **Button press**: Cycles through `currentImageIndex`
- **Wraparound**: After last image, returns to first
- **Debouncing**: 300ms delay prevents multiple triggers
- **Visual feedback**: LED blinks to show system responsiveness

## File Structure

```
ESP32-S3-GEEK_SD_JPEG_PLAYER/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp            # Main application code
│   ├── DEV_Config.h        # Hardware pin definitions
│   ├── LCD_Driver.cpp/h    # ST7789V display driver
│   ├── GUI_Paint.cpp/h     # Graphics and drawing functions
│   └── image.h             # Embedded image configuration (optional)
├── scripts/
│   └── convert_images.py   # Convert images to embedded format
├── lib/
│   └── TFT_eSPI/           # Display library configuration
├── EMBEDDED_IMAGES_GUIDE.md # How to add embedded images
└── README.md               # This documentation
```

## Dependencies

- **PlatformIO Framework**: Arduino-ESP32
- **JPEGDEC Library**: bitbank2/JPEGDEC@^1.3.3 (for SD card JPEG decoding)
- **Built-in Libraries**: SPI, SD, FS

## Usage Instructions

### 1. **Setup SD Card**
- Format microSD card as FAT32
- Copy JPEG files to root directory (not in folders)
- Supported formats: `.jpg`, `.jpeg`
- Any resolution (will be auto-scaled)

### 2. **Optional: Add Embedded Images**
- See `EMBEDDED_IMAGES_GUIDE.md` for detailed instructions
- Perfect for logos, icons, or boot screens
- Always available without SD card

### 3. **Upload Firmware**
```bash
# Build and upload
pio run --target upload

# Monitor serial output (replace COMx with your device's port)
pio device monitor --port COMx --baud 115200
# Or let PlatformIO auto-detect the port:
pio device monitor --baud 115200
```

### 4. **Operation**
1. Power on ESP32-S3 Geek
2. Wait for initialization (LED starts blinking)
3. If images found, first image displays automatically
4. If no images: "No Images Found" message displays
5. Press BOOT button to cycle through available images
6. Serial monitor shows detailed loading/scaling info

## Technical Specifications

- **Display**: 240x135 pixels, RGB565 color format
- **Memory**: ~65KB per full-size image (135x240x2 bytes)
- **SD Card**: SPI interface at 80MHz
- **Image Processing**: Hardware-accelerated JPEG decode with software scaling
- **Rotation**: 270° clockwise transformation for correct orientation
- **Performance**: Instant image switching (pre-loaded), ~2-3 seconds JPEG loading

## Troubleshooting

### SD Card Issues
- Ensure FAT32 format (not exFAT or NTFS)
- Check wiring: SCK=36, MISO=37, MOSI=35, SS=34
- Use high-quality SD card (Class 10 recommended)
- Maximum tested: 32GB SD cards

### Image Issues
- **Corrupted display**: Check JPEG file integrity
- **Wrong colors**: Ensure RGB565 format in embedded images
- **Memory errors**: Reduce number of large JPEG files
- **Rotation problems**: Verify 270° clockwise is correct for your setup

### Build Issues
- Update PlatformIO to latest version
- Clean build: `pio run --target clean`
- Check ESP32-S3 board selection in platformio.ini

## Serial Monitor Output Example

```
===================================
ESP32-S3 Geek LCD_BUTTON Program
===================================
✅ Hardware initialized
✅ LCD initialized successfully!
✅ Paint initialized  
📸 Initializing embedded images...
✅ 3 embedded images initialized
✅ SD Card initialized successfully!
🔍 Loading images from SD card...
📖 Loading JPEG: vacation.jpg
📏 Original size: 1920x1080
📐 Target size (after rotation): 135x76
📍 Offset: (0, 82)
✅ Scaled, letterboxed, and rotated 270° clockwise
✅ Loaded JPEG: vacation.jpg
📁 All available images:
  1. pic1.png (embedded)
  2. pic2.png (embedded)  
  3. profile.jpg (embedded)
  4. vacation.jpg (SD card)
🎉 LCD_BUTTON PROGRAM READY!
📸 Total images available: 4
Press BOOT button to cycle through all images!
```

## Embedded Images Recommendations

The current embedded images include:
1. **pic1_data.h** - Sample image 1
2. **pic2_data.h** - Sample image 2  
3. **img_544884888...** - User profile photo

### Suggested Embedded Images:
- **Logo/Branding**: Company or project logo
- **Default/Placeholder**: Generic "No SD Card" or "Insert Images" message
- **Demo Images**: High-quality sample photos to showcase capabilities
- **System Icons**: Battery status, WiFi icons, menu graphics
- **Boot Screen**: Startup logo or system information

### Converting Images to Embedded Format:
Use the included conversion scripts in `/scripts/` directory:
```bash
python convert_images.py your_image.jpg pic3_data.h
```

## License

This project is based on Waveshare ESP32-S3 Geek examples and uses open-source libraries. Feel free to modify and distribute.

## Version History

- **v1.0** - Basic embedded image cycling
- **v2.0** - Added SD card support and JPEG decoding
- **v3.0** - Implemented proper scaling, letterboxing, and rotation
- **v3.1** - Final release with complete documentation