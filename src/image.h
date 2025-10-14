#ifndef _IMAGE_H_
#define _IMAGE_H_

// Uncomment the line below to enable embedded images
// #define ENABLE_EMBEDDED_IMAGES

// Status images are always included for system feedback
#include "no_sd_card_data.h"
#include "no_images_found_data.h"
#include "scanning_data.h"
#include "manual_mode_data.h"
#include "slideshow_mode_data.h"

#ifdef ENABLE_EMBEDDED_IMAGES
// When enabled, place your image data files (e.g., pic1_data.h) in src/ directory
// and include them here:
// #include "pic1_data.h"
// #include "pic2_data.h" 
// #include "your_image_data.h"
#endif

#endif