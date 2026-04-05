#pragma once
#include <pebble.h>

typedef struct ClaySettings {
  bool ShortList;
} ClaySettings;

extern ClaySettings settings;

void load_settings(void);
void save_settings(void);
void default_settings(void);
