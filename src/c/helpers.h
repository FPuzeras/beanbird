#pragma once
#include <stdint.h>
#include <math.h>

#define STORAGE_KEY_SETTINGS 1
#define STORAGE_KEY_INGESTION 2
#define STORAGE_KEY_INGESTION_META 3

#ifdef PBL_PLATFORM_EMERY 
  #define CENTER_TEXT_OFFSET_H 80
  #define TOP_TEXT_OFFSET_H_1 0
  #define TOP_TEXT_OFFSET_H_2 26
  #define CENTER_TEXT_FONT FONT_KEY_LECO_60_NUMBERS_AM_PM
  #define CENTER_TEXT_SIZE_H 60
  #define AUX_TEXT_FONT FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM
  #define AUX_TEXT_SIZE_H 26
#else
  #define CENTER_TEXT_OFFSET_H 70
  #define TOP_TEXT_OFFSET_H_1 0
  #define TOP_TEXT_OFFSET_H_2 20
  #define CENTER_TEXT_FONT FONT_KEY_LECO_42_NUMBERS
  #define CENTER_TEXT_SIZE_H 42
  #define AUX_TEXT_FONT FONT_KEY_LECO_20_BOLD_NUMBERS
  #define AUX_TEXT_SIZE_H 20
#endif

static inline uint16_t float_to_u16(float f) {
    if (isnan(f)) return 0;
    if (f <= 0.0f) return 0;
    if (f >= 65535.0f) return 65535;
    return (uint16_t)lroundf(f);
}

static inline float get_ke_from_half_life(int16_t half_life_min) {
    if (half_life_min <= 0) return 0.0f;
    
    // ln(2) approx 0.693147f
    return 0.69314718f / (half_life_min * 60);
}

static inline float get_ka_from_half_life(int16_t abs_half_life_min) {
    if (abs_half_life_min <= 0) return 0.0f;

    // ln(2) approx 0.693147f
    return 0.69314718f / (abs_half_life_min * 60);
}