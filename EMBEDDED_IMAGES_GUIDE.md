# Embedded Images Guide

This guide explains how to add embedded images to your ESP32-S3 Geek project. By default, the project only uses SD card images, but you can easily add embedded images for logos, icons, or critical graphics.

## Why Use Embedded Images?

### Advantages:
- ✅ **Always Available**: Work without SD card
- ✅ **Instant Loading**: No file system delays
- ✅ **Boot Screens**: Perfect for logos and startup images
- ✅ **Reliability**: No file corruption issues
- ✅ **Offline Operation**: Self-contained system

### Disadvantages:
- ❌ **Limited Space**: Flash memory is finite
- ❌ **Compilation Time**: Increases build time
- ❌ **Fixed Content**: Can't change without reflashing
- ❌ **Code Size**: Makes firmware larger

## Quick Start: Adding Your First Embedded Image

**Simple 4-step process:**

### Step 1: Convert Your Image
```bash
cd scripts/
python convert_images.py my_logo.png logo_data.h
```

### Step 2: Copy to Source Directory
```bash
cp logo_data.h ../src/
```

### Step 3: Enable Embedded Images
Edit `src/image.h`:
```cpp
#ifndef _IMAGE_H_
#define _IMAGE_H_

// Uncomment the line below to enable embedded images
#define ENABLE_EMBEDDED_IMAGES  // ← Uncomment this line

#ifdef ENABLE_EMBEDDED_IMAGES
#include "logo_data.h"          // ← Add this line
#endif

#endif
```

### Step 4: Add Image to Code
Edit `src/main.cpp` in the `initializeEmbeddedImages()` function:
```cpp
#ifdef ENABLE_EMBEDDED_IMAGES
  Serial.println("📸 Initializing embedded images...");
  
  // Add logo image
  imageList[totalImages].fileName = "logo.png";
  imageList[totalImages].isEmbedded = true;
  imageList[totalImages].embeddedData = logo_data;
  imageList[totalImages].sdData = nullptr;
  imageList[totalImages].width = logo_data_width;
  imageList[totalImages].height = logo_data_height;
  totalImages++;
  
  Serial.println("✅ " + String(totalImages) + " embedded images initialized");
#endif
```

That's it! Build and upload your firmware.

## When to Use Embedded Images

**Good Use Cases:**
- Company/project logos
- System status icons (WiFi, battery, etc.)
- Boot/splash screens
- Default "No SD Card" placeholders
- Critical UI elements
- Demo/sample images

**Avoid For:**
- Large photo collections
- Frequently changing content
- User-generated images
- Temporary displays

## Image Conversion Script

The `scripts/convert_images.py` script handles all the technical details:

### Basic Usage:
```bash
cd scripts/
python convert_images.py input_image.png output_data.h
```

### Advanced Options:
```bash
# Custom size
python convert_images.py icon.png icon_data.h 64 64

# Batch conversion
for file in *.png; do
    python convert_images.py "$file" "${file%.png}_data.h"
done
```

### What the Script Does:
1. **Loads** the input image
2. **Resizes** to fit display (maintains aspect ratio)
3. **Converts** to RGB565 format (16-bit color)
4. **Rotates** 270° clockwise for correct orientation
5. **Generates** C header file with pixel data

## Memory Considerations

### Flash Memory Usage:
- **135x240 image**: ~65KB (full screen)
- **64x64 icon**: ~8KB (small icon)
- **ESP32-S3 flash**: 8MB total (plenty of space)

### Recommended Limits:
- **Total embedded images**: < 500KB
- **Individual image**: < 100KB  
- **Number of images**: < 10 for reasonable build times

## Multiple Images Example

If you want to add several embedded images:

### Step 1: Convert All Images
```bash
cd scripts/
python convert_images.py company_logo.png logo_data.h
python convert_images.py boot_screen.jpg boot_data.h
python convert_images.py wifi_icon.png wifi_data.h
```

### Step 2: Update image.h
```cpp
#ifndef _IMAGE_H_
#define _IMAGE_H_

#define ENABLE_EMBEDDED_IMAGES

#ifdef ENABLE_EMBEDDED_IMAGES
#include "logo_data.h"
#include "boot_data.h"
#include "wifi_data.h"
#endif

#endif
```

### Step 3: Update main.cpp
```cpp
#ifdef ENABLE_EMBEDDED_IMAGES
  Serial.println("📸 Initializing embedded images...");
  
  // Company logo
  imageList[totalImages].fileName = "company_logo.png";
  imageList[totalImages].isEmbedded = true;
  imageList[totalImages].embeddedData = logo_data;
  imageList[totalImages].sdData = nullptr;
  imageList[totalImages].width = logo_data_width;
  imageList[totalImages].height = logo_data_height;
  totalImages++;
  
  // Boot screen
  imageList[totalImages].fileName = "boot_screen.jpg";
  imageList[totalImages].isEmbedded = true;
  imageList[totalImages].embeddedData = boot_data;
  imageList[totalImages].sdData = nullptr;
  imageList[totalImages].width = boot_data_width;
  imageList[totalImages].height = boot_data_height;
  totalImages++;
  
  // WiFi icon
  imageList[totalImages].fileName = "wifi_icon.png";
  imageList[totalImages].isEmbedded = true;
  imageList[totalImages].embeddedData = wifi_data;
  imageList[totalImages].sdData = nullptr;
  imageList[totalImages].width = wifi_data_width;
  imageList[totalImages].height = wifi_data_height;
  totalImages++;
  
  Serial.println("✅ " + String(totalImages) + " embedded images initialized");
#endif
```

## Troubleshooting

### Build Errors:
**"Variable not declared"**
- Make sure `#define ENABLE_EMBEDDED_IMAGES` is uncommented
- Check that all included header files exist in `src/` directory
- Verify include statements in `image.h`

**"Out of memory"**
- Reduce image sizes or count
- Use lower resolution images
- Consider using SD card for large collections

### Display Issues:
**Images appear corrupted**
- Verify conversion script completed successfully
- Check that width/height constants are used correctly
- Ensure proper RGB565 format

**Wrong colors**
- Re-run conversion script
- Check original image format
- Verify RGB565 color space

## Best Practices

### Image Preparation:
1. **Optimize size**: Resize to target resolution (135x240) before conversion
2. **Use appropriate format**: PNG for graphics, JPG for photos
3. **Test individual images**: Convert and test one at a time
4. **Keep originals**: Save source images for future modifications

### Development Workflow:
1. **Start small**: Begin with one essential image (logo)
2. **Test incrementally**: Add one image at a time
3. **Document purpose**: Keep track of what each image is for
4. **Version control**: Include both source images and generated headers

### Hybrid Approach:
- **Embedded**: Logos, icons, critical UI elements (< 10 images)
- **SD Card**: Photos, user content, large collections
- **Fallback**: Use embedded "No SD Card" message when SD fails

## Advanced Features

### Conditional Compilation:
```cpp
#ifdef COMPANY_BRANDING
  #include "company_logo.h"
#endif

#ifdef DEBUG_MODE
  #include "debug_screen.h"
#endif
```

### Multiple Image Sets:
```cpp
#ifdef PRODUCT_A
  #include "product_a_logo.h"
#elif defined(PRODUCT_B)
  #include "product_b_logo.h"
#endif
```

## Example: Company Branding Setup

Perfect for adding your company logo that displays on startup:

### 1. Prepare Your Logo
- Format: PNG or JPG
- Recommended size: 135x240 pixels (or smaller for centered logo)
- Keep file size reasonable (< 1MB for faster conversion)

### 2. Convert and Install
```bash
cd scripts/
python convert_images.py ../my_company_logo.png company_logo_data.h
cp company_logo_data.h ../src/
```

### 3. Update Code
**Edit `src/image.h`:**
```cpp
#define ENABLE_EMBEDDED_IMAGES
#include "company_logo_data.h"
```

**Edit `src/main.cpp`:**
```cpp
// In initializeEmbeddedImages() function:
imageList[totalImages].fileName = "Company Logo";
imageList[totalImages].isEmbedded = true;
imageList[totalImages].embeddedData = company_logo_data;
imageList[totalImages].sdData = nullptr;
imageList[totalImages].width = company_logo_data_width;
imageList[totalImages].height = company_logo_data_height;
totalImages++;
```

### 4. Build and Test
```bash
pio run --target upload
pio device monitor
```

Now your company logo will always be available, even without an SD card!

## Summary

Embedded images provide reliable, fast-loading graphics for essential UI elements. Use them strategically for logos, icons, and critical graphics while leveraging SD card storage for larger, dynamic content.

**Key Points:**
- Only enable when needed (`#define ENABLE_EMBEDDED_IMAGES`)
- Use conversion script for proper format
- Start with one image and test
- Keep total embedded images reasonable (< 500KB)
- Perfect for branding and critical UI elements

For more examples, check the `backup_working_version/` directory which contains a working embedded images setup.