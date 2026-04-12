#pragma once
#include <pebble.h>
#include <stdint.h>

#define STR_MAXLEN 15
#define DRINK_COUNT 6

typedef struct ClaySettings {
  int16_t caff_content[DRINK_COUNT];
  char drink_titles[DRINK_COUNT][STR_MAXLEN + 1];
  
  int8_t drink_time[3];
  
  int16_t custom_caff;
  int8_t custom_step;
  int16_t sleep_mg;
  
  int16_t half_life_elim;
  int8_t half_life_abs;
  
  #ifdef PBL_COLOR
    int16_t minimum_mg;
    int16_t maximum_mg;
  #endif
} ClaySettings;

extern ClaySettings settings;

void parse_inbox_settings(DictionaryIterator *iter);

void load_settings(void);
