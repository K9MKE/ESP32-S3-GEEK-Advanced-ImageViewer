# ESP32-S3 Geek Module LCD_BUTTON Project

This workspace is configured for developing firmware for the Waveshare ESP32-S3 Geek module, specifically the LCD_BUTTON program that demonstrates LCD display and button functionality.

## Project Specifications
- **Hardware**: Waveshare ESP32-S3 Geek module
- **Display**: Built-in LCD screen (ST7789V 1.14" 240x135)
- **Input**: Physical buttons (BOOT button on GPIO0)
- **Development**: PlatformIO with Arduino framework
- **Language**: C/C++ for embedded systems

## Development Guidelines
- Use ESP32-S3 specific libraries and configurations
- Follow Waveshare's hardware pin mappings
- Implement proper LCD initialization and button handling
- Use efficient memory management for embedded constraints
- Follow ESP-IDF or Arduino framework conventions

## Hardware Pin Configuration
- TFT_CS: GPIO10
- TFT_RST: GPIO14  
- TFT_DC: GPIO13
- TFT_MOSI: GPIO11
- TFT_SCLK: GPIO12
- BUTTON_PIN: GPIO0 (BOOT button)
- LED_PIN: GPIO2 (built-in LED)

## Hardware Features
- ESP32-S3 dual-core processor
- Built-in ST7789V LCD display (240x135 pixels)
- Physical buttons for user interaction
- Wi-Fi and Bluetooth capabilities
- GPIO pins for expansion

## Code Style
- Use descriptive variable names for hardware pins
- Comment hardware-specific configurations
- Follow embedded C/C++ best practices
- Use proper delay functions for timing-sensitive operations