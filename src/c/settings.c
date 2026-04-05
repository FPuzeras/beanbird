#include <pebble.h>
#include "src/c/settings.h"
#include "src/c/storage.h"

ClaySettings settings;

static void prv_default_settings() {
  // caff_content
  static const int16_t default_caff[DRINK_COUNT] = { 1, 2, 3, 4, 5, 6 };
  memcpy(settings.caff_content, default_caff, sizeof default_caff);
  
  // drink_title
  const char *default_titles[DRINK_COUNT] = {
    "one", "two", "three", "four", "five", "six"
  };
  for (size_t i = 0; i < DRINK_COUNT; ++i) {
      strncpy(settings.drink_titles[i], default_titles[i], STR_MAXLEN);
      settings.drink_titles[i][STR_MAXLEN] = '\0';
  }
}

static void prv_save_settings() {
  persist_write_data(STORAGE_KEY_SETTINGS, &settings, sizeof(settings));
}

static void prv_load_settings() {
  prv_default_settings();
  persist_read_data(STORAGE_KEY_SETTINGS, &settings, sizeof(settings));
}

void load_settings(void) {
  prv_load_settings();
}

void parse_inbox_settings(DictionaryIterator *iter) {
  // load drinks
  for (uint8_t i = 0; i < DRINK_COUNT; i++) {
    Tuple *tuple = dict_find(iter, MESSAGE_KEY_CaffContent + i);
    if (tuple) {
      settings.caff_content[i] = tuple->value->int16;
    }
    tuple = dict_find(iter, MESSAGE_KEY_DrinkTitle + i);
    strncpy(settings.drink_titles[i], tuple->value->cstring, STR_MAXLEN);
    settings.drink_titles[i][STR_MAXLEN] = '\0';
  }
  
  prv_save_settings();
}