#!/usr/bin/env python3
"""
FlipperZero-Style Status Image Generator for ESP32-S3
Creates professional status screens with orange/black color scheme
Target: 240x135 pixels (landscape orientation)
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_flipper_style_image(width, height, title, icon_type, subtitle=""):
    """Create a FlipperZero-style status image with orange/black theme"""
    
    # FlipperZero color scheme - Orange on black
    bg_color = (0, 0, 0)        # Black background
    fg_color = (255, 140, 0)    # FlipperZero orange
    
    # Create image
    img = Image.new('RGB', (width, height), bg_color)
    draw = ImageDraw.Draw(img)
    
    # Try to load a font, fallback to default
    try:
        # Try to find a monospace font
        font_large = ImageFont.truetype("consola.ttf", 16)
        font_medium = ImageFont.truetype("consola.ttf", 12)
        font_small = ImageFont.truetype("consola.ttf", 10)
    except:
        font_large = ImageFont.load_default()
        font_medium = ImageFont.load_default()
        font_small = ImageFont.load_default()
    
    # Draw the main icon
    icon_x = width // 2
    icon_y = 30
    
    if icon_type == "no_sd":
        # Draw SD card with X
        # SD card outline
        card_w, card_h = 40, 30
        card_x = icon_x - card_w//2
        card_y = icon_y - card_h//2
        
        # SD card shape
        draw.rectangle([card_x, card_y, card_x + card_w, card_y + card_h], outline=fg_color, width=2)
        draw.rectangle([card_x + card_w - 8, card_y - 4, card_x + card_w, card_y + 4], outline=fg_color, width=2)
        
        # Big X over it
        draw.line([card_x - 5, card_y - 5, card_x + card_w + 5, card_y + card_h + 5], fill=fg_color, width=3)
        draw.line([card_x + card_w + 5, card_y - 5, card_x - 5, card_y + card_h + 5], fill=fg_color, width=3)
        
    elif icon_type == "no_images":
        # Draw folder with X
        folder_w, folder_h = 36, 28
        folder_x = icon_x - folder_w//2
        folder_y = icon_y - folder_h//2
        
        # Folder shape
        draw.rectangle([folder_x, folder_y + 6, folder_x + folder_w, folder_y + folder_h], outline=fg_color, width=2)
        draw.rectangle([folder_x, folder_y, folder_x + 12, folder_y + 8], outline=fg_color, width=2)
        
        # X over it
        draw.line([folder_x - 5, folder_y - 2, folder_x + folder_w + 5, folder_y + folder_h + 2], fill=fg_color, width=3)
        draw.line([folder_x + folder_w + 5, folder_y - 2, folder_x - 5, folder_y + folder_h + 2], fill=fg_color, width=3)
        
    elif icon_type == "scanning":
        # Draw animated scanning bars
        bar_width = 6
        bar_spacing = 8
        bars_total = 5
        start_x = icon_x - (bars_total * (bar_width + bar_spacing)) // 2
        
        for i in range(bars_total):
            bar_x = start_x + i * (bar_width + bar_spacing)
            # Vary height to show animation
            bar_height = 10 + (i % 3) * 8
            bar_y = icon_y - bar_height // 2
            draw.rectangle([bar_x, bar_y, bar_x + bar_width, bar_y + bar_height], fill=fg_color)
            
    elif icon_type == "manual_mode":
        # Draw hand/finger icon for manual control
        # Simple hand representation
        palm_x, palm_y = icon_x - 8, icon_y + 5
        draw.rectangle([palm_x, palm_y, palm_x + 16, palm_y + 20], fill=fg_color)
        # Fingers
        for i in range(4):
            finger_x = palm_x + 2 + i * 3
            draw.rectangle([finger_x, palm_y - 12, finger_x + 2, palm_y], fill=fg_color)
        # Thumb
        draw.rectangle([palm_x - 6, palm_y + 5, palm_x - 2, palm_y + 15], fill=fg_color)
        
    elif icon_type == "slideshow_mode":
        # Draw play button with speed lines
        # Triangle (play button)
        triangle = [(icon_x - 15, icon_y - 12), (icon_x - 15, icon_y + 12), (icon_x + 10, icon_y)]
        draw.polygon(triangle, fill=fg_color)
        
        # Speed lines
        for i in range(3):
            line_x = icon_x + 20 + i * 6
            draw.line([line_x, icon_y - 8, line_x + 8, icon_y - 8], fill=fg_color, width=2)
            draw.line([line_x, icon_y, line_x + 8, icon_y], fill=fg_color, width=2)
            draw.line([line_x, icon_y + 8, line_x + 8, icon_y + 8], fill=fg_color, width=2)
    
    # Draw title text
    title_bbox = draw.textbbox((0, 0), title, font=font_large)
    title_width = title_bbox[2] - title_bbox[0]
    title_x = (width - title_width) // 2
    title_y = icon_y + 35
    draw.text((title_x, title_y), title, fill=fg_color, font=font_large)
    
    # Draw subtitle if provided
    if subtitle:
        subtitle_bbox = draw.textbbox((0, 0), subtitle, font=font_medium)
        subtitle_width = subtitle_bbox[2] - subtitle_bbox[0]
        subtitle_x = (width - subtitle_width) // 2
        subtitle_y = title_y + 25
        draw.text((subtitle_x, subtitle_y), subtitle, fill=fg_color, font=font_medium)
    
    # Draw decorative border
    draw.rectangle([2, 2, width-3, height-3], outline=fg_color, width=1)
    
    return img

def main():
    """Generate all status images"""
    # Correct dimensions for ESP32-S3 display (landscape)
    width, height = 240, 135
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    print("🎨 Generating FlipperZero-style status images...")
    print(f"📐 Target size: {width}x{height} pixels")
    print("🎨 Color scheme: Orange on black (FlipperZero style)")
    
    # Generate status images
    images_to_create = [
        ("no_sd_card.png", "NO SD CARD", "no_sd", "Insert SD card"),
        ("no_images_found.png", "NO IMAGES", "no_images", "Add JPEG files"),
        ("scanning.png", "SCANNING", "scanning", "Please wait..."),
        ("manual_mode.png", "MANUAL MODE", "manual_mode", "Press to advance"),
        ("slideshow_mode.png", "SLIDESHOW", "slideshow_mode", "Auto-advancing"),
    ]
    
    for filename, title, icon_type, subtitle in images_to_create:
        img = create_flipper_style_image(width, height, title, icon_type, subtitle)
        
        output_path = os.path.join(script_dir, filename)
        img.save(output_path)
        print(f"✅ Created: {filename}")
    
    print("\n🎉 All FlipperZero-style images generated!")
    print("🔄 Run convert_images.py next to create header files")

if __name__ == "__main__":
    main()