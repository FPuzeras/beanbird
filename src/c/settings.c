#include <pebble.h>
#include "src/c/settings.h"
#include "src/c/storage.h"

ClaySettings settings;

static void prv_default_settings() {
  settings.BackgroundColor = GColorBlack;
  settings.TextColor = GColorWhite;
  settings.TemperatureUnit = false;
  settings.ShowDate = true;
}

static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_load_settings() {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void load_settings(void) {
  prv_load_settings();
}

void save_settings(void) {
  prv_save_settings();
}

void default_settings(void) {
  prv_default_settings();
}