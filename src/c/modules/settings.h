#pragma once
#include <pebble.h>
#include <stdint.h>

#define STR_MAXLEN 15
#define DRINK_COUNT 6

typedef struct ClaySettings {
  int16_t caff_content[DRINK_COUNT];
  char drink_titles[DRINK_COUNT][STR_MAXLEN + 1];
  uint16_t custom_caff;
  uint16_t custom_step;
  float elimination_constant;
  float absorbtion_constant;
} ClaySettings;

extern ClaySettings settings;

void parse_inbox_settings(DictionaryIterator *iter);

void load_settings(void);
