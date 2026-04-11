#pragma once
#include <pebble.h>
#include <stdint.h>
#include <math.h>

#define STORAGE_KEY_SETTINGS 1
#define STORAGE_KEY_INGESTION 2
#define STORAGE_KEY_INGESTION_META 3

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

static inline void prv_post_caff_added() {
  exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
  window_stack_pop_all(false);
}
