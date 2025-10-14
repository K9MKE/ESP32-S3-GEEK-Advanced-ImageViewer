#!/usr/bin/env python3
"""
Enhanced Image Converter for ESP32-S3 LCD Display
Converts various image formats (PNG, JPG, JPEG, BMP, GIF, WEBP, TIFF) to RGB565 C header files
Handles any input size with smart resizing and aspect ratio preservation
"""

from PIL import Image
import os
import glob

def convert_image_to_rgb565_header(image_path, output_path, var_name):
    """Convert any supported image format to RGB565 C header file"""
    
    try:
        # Open image and convert to RGB (handles transparency in PNG/GIF)
        print(f"Processing: {os.path.basename(image_path)}")
        img = Image.open(image_path)
        
        # Handle animated GIFs - take first frame
        if hasattr(img, 'is_animated') and img.is_animated:
            img.seek(0)  # Go to first frame
            
        # Convert to RGB (removes alpha channel if present)
        if img.mode in ('RGBA', 'LA', 'P'):
            # Create white background for transparent images
            background = Image.new('RGB', img.size, (255, 255, 255))
            if img.mode == 'P':
                img = img.convert('RGBA')
            background.paste(img, mask=img.split()[-1] if img.mode in ('RGBA', 'LA') else None)
            img = background
        else:
            img = img.convert('RGB')
        
        print(f"Original size: {img.size[0]}x{img.size[1]}")
        
        # Smart resize with aspect ratio preservation
        img = smart_resize_for_display(img, 240, 135)
        print(f"Resized to: {img.size[0]}x{img.size[1]}")
        
        # Rotate 90 degrees clockwise to get 135x240 for display
        img = img.rotate(-90, expand=True)
        
        # Final dimensions after rotation
        width, height = img.size
        print(f"Final display size: {width}x{height}")
        
        # Convert to RGB565 format
        pixels = []
        for y in range(height):
            for x in range(width):
                r, g, b = img.getpixel((x, y))
                # Convert to RGB565 (5-6-5 bits)
                r565 = (r >> 3) << 11
                g565 = (g >> 2) << 5
                b565 = b >> 3
                rgb565 = r565 | g565 | b565
                pixels.append(rgb565)
        
        # Generate C header file
        with open(output_path, 'w') as f:
            f.write(f"// Auto-generated from {os.path.basename(image_path)}\n")
            f.write(f"// Original size: {img.size[0]}x{img.size[1]} -> Display size: {width}x{height} pixels\n")
            f.write(f"// Format: RGB565, rotated for ESP32-S3 Geek display\n\n")
            f.write(f"#ifndef {var_name.upper()}_H\n")
            f.write(f"#define {var_name.upper()}_H\n\n")
            f.write(f"#include <Arduino.h>\n\n")
            f.write(f"const uint16_t {var_name}_width = {width};\n")
            f.write(f"const uint16_t {var_name}_height = {height};\n\n")
            f.write(f"const uint16_t {var_name}[] PROGMEM = {{\n")
            
            # Write pixel data in rows of 12 values
            for i in range(0, len(pixels), 12):
                row = pixels[i:i+12]
                row_str = ", ".join(f"0x{pixel:04X}" for pixel in row)
                f.write(f"    {row_str}")
                if i + 12 < len(pixels):
                    f.write(",")
                f.write("\n")
            
            f.write(f"}};\n\n")
            f.write(f"#endif // {var_name.upper()}_H\n")
        
        print(f"✅ Successfully converted: {os.path.basename(image_path)} -> {os.path.basename(output_path)}")
        return True
        
    except Exception as e:
        print(f"❌ Error processing {os.path.basename(image_path)}: {str(e)}")
        return False

def smart_resize_for_display(img, target_width, target_height):
    """
    Smart resize that preserves aspect ratio and fits image optimally
    Options: 'fit' (letterbox), 'fill' (crop), 'stretch' (distort)
    """
    original_width, original_height = img.size
    
    # Calculate aspect ratios
    original_ratio = original_width / original_height
    target_ratio = target_width / target_height
    
    print(f"Aspect ratios - Original: {original_ratio:.2f}, Target: {target_ratio:.2f}")
    
    # Option 1: Fit (letterbox) - preserves entire image, may add borders
    if original_ratio > target_ratio:
        # Image is wider than target - fit to width
        new_width = target_width
        new_height = int(target_width / original_ratio)
    else:
        # Image is taller than target - fit to height  
        new_height = target_height
        new_width = int(target_height * original_ratio)
    
    # Resize the image
    img_resized = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
    
    # Create final image with target dimensions and center the resized image
    final_img = Image.new('RGB', (target_width, target_height), (0, 0, 0))  # Black background
    
    # Calculate position to center the image
    x_offset = (target_width - new_width) // 2
    y_offset = (target_height - new_height) // 2
    
    final_img.paste(img_resized, (x_offset, y_offset))
    
    print(f"Resize strategy: Fit with centering (letterbox if needed)")
    
    return final_img

def get_supported_images(directory):
    """Find all supported image files in directory"""
    supported_extensions = ['*.png', '*.jpg', '*.jpeg', '*.bmp', '*.gif', '*.webp', '*.tiff', '*.tif']
    image_files = []
    
    for extension in supported_extensions:
        pattern = os.path.join(directory, extension)
        image_files.extend(glob.glob(pattern, recursive=False))
        # Also check uppercase
        pattern_upper = os.path.join(directory, extension.upper())
        image_files.extend(glob.glob(pattern_upper, recursive=False))
    
    return sorted(list(set(image_files)))  # Remove duplicates and sort
    """Convert PNG to RGB565 C header file"""
    
    # Open image and convert to RGB
    img = Image.open(png_path)
    img = img.convert('RGB')
    
    # First resize to 240x135 (opposite of what we want)
    img = img.resize((240, 135), Image.Resampling.LANCZOS)
    
    # Then rotate 90 degrees clockwise to get 135x240
    img = img.rotate(-90, expand=True)
    
    # Now the image should be 135x240
    width, height = img.size
    print(f"Final image size: {width}x{height}")
    
    # Convert to RGB565 format
    pixels = []
    for y in range(height):
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            # Convert to RGB565 (5-6-5 bits)
            r565 = (r >> 3) << 11
            g565 = (g >> 2) << 5
            b565 = b >> 3
            rgb565 = r565 | g565 | b565
            pixels.append(rgb565)
    
    # Generate C header file
    with open(output_path, 'w') as f:
        f.write(f"// Auto-generated from {os.path.basename(png_path)}\n")
        f.write(f"// Image size: {width}x{height} pixels, RGB565 format\n\n")
        f.write(f"#ifndef {var_name.upper()}_H\n")
        f.write(f"#define {var_name.upper()}_H\n\n")
        f.write(f"#include <Arduino.h>\n\n")
        f.write(f"const uint16_t {var_name}_width = {width};\n")
        f.write(f"const uint16_t {var_name}_height = {height};\n\n")
        f.write(f"const uint16_t {var_name}[] PROGMEM = {{\n")
        
        # Write pixel data in rows of 12 values
        for i in range(0, len(pixels), 12):
            row = pixels[i:i+12]
            row_str = ", ".join(f"0x{pixel:04X}" for pixel in row)
            f.write(f"    {row_str}")
            if i + 12 < len(pixels):
                f.write(",")
            f.write("\n")
        
        f.write(f"}};\n\n")
        f.write(f"#endif // {var_name.upper()}_H\n")
    
    print(f"Converted {png_path} -> {output_path}")

def main():
    """Convert all supported image files in project directory"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    print("🖼️  Enhanced Image Converter for ESP32-S3 LCD Display")
    print("=" * 60)
    print("Supported formats: PNG, JPG, JPEG, BMP, GIF, WEBP, TIFF")
    print("Target display: 135x240 pixels (ST7789V)")
    print("=" * 60)
    
    # Find all supported image files in scripts directory first, then project root
    image_files = get_supported_images(script_dir)
    if not image_files:
        image_files = get_supported_images(project_root)
    
    if not image_files:
        print("❌ No supported image files found in project root")
        print("   Supported formats: .png, .jpg, .jpeg, .bmp, .gif, .webp, .tiff")
        return
    
    print(f"📁 Found {len(image_files)} image file(s):")
    for img_file in image_files:
        print(f"   - {os.path.basename(img_file)}")
    print()
    
    # Convert each image
    success_count = 0
    for image_path in image_files:
        # Generate variable name from filename
        base_name = os.path.splitext(os.path.basename(image_path))[0]
        # Clean up variable name (remove special characters)
        var_name = "".join(c if c.isalnum() else "_" for c in base_name).lower()
        if var_name[0].isdigit():
            var_name = "img_" + var_name
        var_name += "_data"
        
        # Output header file
        header_path = os.path.join(script_dir, f"{var_name}.h")
        
        # Convert the image
        if convert_image_to_rgb565_header(image_path, header_path, var_name):
            success_count += 1
            print(f"   📄 Generated: {os.path.basename(header_path)}")
        print()
    
    print("=" * 60)
    print(f"✅ Conversion complete: {success_count}/{len(image_files)} files processed")
    print(f"📂 Header files saved to: {script_dir}")
    print()
    print("💡 Usage in your code:")
    print('   #include "your_image_data.h"')
    print('   Paint_DrawImage(your_image_data, 0, 0, your_image_data_width, your_image_data_height);')
    print()
    print("🔧 To use in src/ directory:")
    print("   Copy the .h files from scripts/ to src/ folder")

if __name__ == "__main__":
    main()