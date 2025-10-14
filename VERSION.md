# Version History

## v2.0 - Current Release
**Version**: 2.0.0  
**Release Date**: October 14, 2025  
**Status**: Stable Release

### 🎯 **Project Overview**
ESP32-S3 GEEK Image Display is a comprehensive solution for displaying JPEG images from SD card on the Waveshare ESP32-S3 GEEK module. Features automatic image processing, scaling, and rotation for optimal display quality.

### ✨ **Key Features**
- **SD Card Primary**: Main image source is SD card JPEG files
- **Optional Embedded**: Easy to add logos/icons when needed
- **Image Processing**: Auto-scaling, letterboxing, and rotation
- **Hardware Integration**: Full ESP32-S3 GEEK module support
- **Clean Architecture**: Modular, maintainable codebase

### 📁 **Project Structure**
```
ESP32-S3-GEEK_SD_JPEG_PLAYER/
├── src/                     # Core source code
│   ├── main.cpp            # Main application logic
│   ├── image.h             # Image configuration
│   ├── LCD_Driver.*        # Display drivers
│   ├── GUI_Paint.*         # Graphics functions
│   └── DEV_Config.h        # Hardware configuration
├── scripts/
│   └── convert_images.py   # Image conversion utility
├── EMBEDDED_IMAGES_GUIDE.md # Complete embedded images guide
├── README.md               # Project documentation
└── platformio.ini          # Build configuration
```

### 🖼️ **Image Support**
| Source | Status | Formats | Features |
|--------|--------|---------|----------|
| **SD Card** | ✅ Primary | JPEG/JPG | Auto-loading, scaling, rotation |
| **Embedded** | 🔧 Optional | RGB565 | Disabled by default, guide available |
| **Error Handling** | ✅ Built-in | N/A | Red screen when no images found |

### 🔧 **Technical Specifications**
- **Hardware**: Waveshare ESP32-S3 GEEK module
- **Display**: ST7789V 240x135 pixels
- **Storage**: SD card (FAT32 format)
- **Memory**: Efficient RAM usage with dynamic loading
- **Processing**: Hardware-accelerated JPEG decoding
- **Interface**: Button control with LED status indicator

### 📋 **Dependencies**
- **JPEGDEC**: v1.8.3 for JPEG decoding
- **ESP32 Arduino**: Built-in SPI, SD, FS libraries
- **PlatformIO**: Build system and framework

### 🎯 **Use Cases**
1. **Digital Photo Frame**: Automatic slideshow from SD card
2. **Product Displays**: Show product images or documentation
3. **Development Platform**: Base for custom image applications
4. **Corporate Signage**: Add company logos via embedded images

### 🚀 **Getting Started**
1. **Hardware Setup**: Insert SD card with JPEG files
2. **Software**: Upload firmware with PlatformIO
3. **Operation**: Press BOOT button to cycle through images
4. **Customization**: Follow embedded images guide for logos/icons

### 💡 **Architecture Benefits**
- **Modular Design**: Easy to extend and modify
- **Clean Code**: Well-documented and maintainable
- **Error Handling**: Robust failure recovery
- **Memory Efficient**: Optimized for embedded systems
- **User Friendly**: Simple operation and setup

---

## v1.x - Previous Development
- **v1.0**: Initial embedded image display
- **v1.5**: Added SD card JPEG support
- **v1.8**: Enhanced image processing algorithms
- **v1.9**: Improved memory management and error handling

**Note**: v2.0 represents a mature, production-ready implementation focused on reliability and ease of use.