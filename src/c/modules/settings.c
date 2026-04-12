#include <pebble.h>
#include "src/c/modules/settings.h"
#include "src/c/modules/caffeine.h"
#include "src/c/helpers.h"

ClaySettings settings;

static bool prv_parse_tuple_int8(Tuple *t, int8_t *value) {
  if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
    *value = (int8_t)t->value->uint32;
    return true;
  } else if (t->type == TUPLE_INT) {
    *value = (int8_t)t->value->int32;
    return true;
  } else if (t->type == TUPLE_CSTRING) {
    *value = atoi(t->value->cstring);
    return true;
  }
  return false;
}

static bool prv_parse_tuple_int16(Tuple *t, int16_t *value) {
  if (t->type == TUPLE_INT || t->type == TUPLE_UINT) {
    *value = (int16_t)t->value->uint32;
    return true;
  } else if (t->type == TUPLE_INT) {
    *value = (int16_t)t->value->int32;
    return true;
  } else if (t->type == TUPLE_CSTRING) {
    *value = atoi(t->value->cstring);
    return true;
  }
  return false;
}

static void prv_default_settings() {
  // caff_content
  static const int16_t default_caff[DRINK_COUNT] = { 50, 90, 150, 80, 113, 160 };
  memcpy(settings.caff_content, default_caff, sizeof default_caff);
  
  // drink_title
  const char *default_titles[DRINK_COUNT] = {
    "Tea", "Coffee (M)", "Coffee (L)", "RedBull 250ml", "RedBull 355ml", "RedBull 500ml"
  };
  for (size_t i = 0; i < DRINK_COUNT; ++i) {
      strncpy(settings.drink_titles[i], default_titles[i], STR_MAXLEN);
      settings.drink_titles[i][STR_MAXLEN] = '\0';
  }
  
  settings.custom_caff = 100;
  settings.custom_step = 5;
  
  settings.sleep_mg = 50;
  
  settings.drink_time[0] = 5;
  settings.drink_time[1] = 15;
  settings.drink_time[2] = 30;
  
  settings.half_life_elim = 300;
  settings.half_life_abs = 20;
  
  #ifdef PBL_COLOR
    settings.minimum_mg = 130;
    settings.maximum_mg = 170;
  #endif
}

static void prv_save_settings() {
  persist_write_data(STORAGE_KEY_SETTINGS, &settings, sizeof(settings));
}

static void prv_load_settings() {
  prv_default_settings();
  if (persist_exists(STORAGE_KEY_SETTINGS))
    persist_read_data(STORAGE_KEY_SETTINGS, &settings, sizeof(settings));
}

void load_settings(void) {
  prv_load_settings();
}

void parse_inbox_settings(DictionaryIterator *iter) {
  Tuple *tuple;
  bool recalculate = false;
  
  // load drinks
  for (uint32_t i = 0; i < DRINK_COUNT; i++) {
    tuple = dict_find(iter, MESSAGE_KEY_CaffContent + i);
    if (tuple)
      prv_parse_tuple_int16(tuple, &settings.caff_content[i]);
    
    tuple = dict_find(iter, MESSAGE_KEY_DrinkTitle + i);
    if (tuple) {
      strncpy(settings.drink_titles[i], tuple->value->cstring, STR_MAXLEN);
      settings.drink_titles[i][STR_MAXLEN] = '\0';
    }
  }
  
  // load drink timings
  tuple = dict_find(iter, MESSAGE_KEY_DrinkTiming + 0);
  if (tuple)
    prv_parse_tuple_int8(tuple, &settings.drink_time[0]);
  
  tuple = dict_find(iter, MESSAGE_KEY_DrinkTiming + 1);
  if (tuple)
    prv_parse_tuple_int8(tuple, &settings.drink_time[1]);
  
  tuple = dict_find(iter, MESSAGE_KEY_DrinkTiming + 2);
  if (tuple)
   prv_parse_tuple_int8(tuple, &settings.drink_time[2]);
  
  
  // load custom page preferences
  tuple = dict_find(iter, MESSAGE_KEY_CustomCaff);
  if (tuple) 
    prv_parse_tuple_int16(tuple, &settings.custom_caff);
  
  tuple = dict_find(iter, MESSAGE_KEY_CustomStep);
  if (tuple)
    prv_parse_tuple_int8(tuple, &settings.custom_step);
  
  
  // load model constants
  tuple = dict_find(iter, MESSAGE_KEY_EliminationHL);
  if (tuple) {
    if (prv_parse_tuple_int16(tuple, &settings.half_life_elim)) {
      recalculate = true;
    }
  }
  tuple = dict_find(iter, MESSAGE_KEY_AbsorbtionHL);
  if (tuple) {
    if (prv_parse_tuple_int8(tuple, &settings.half_life_abs)) {
      recalculate = true;
    }
  }
  
  tuple = dict_find(iter, MESSAGE_KEY_SleepMg);
  if (tuple) {
    if (prv_parse_tuple_int16(tuple, &settings.sleep_mg) && !recalculate)
      update_sleep(); // update sleep if mg changed, no need if calculation queued
  }
  
  
  if (recalculate) {
    metabolism_update_settings(settings.half_life_elim, settings.half_life_abs);
  }
  
  
  #ifdef PBL_COLOR
    tuple = dict_find(iter, MESSAGE_KEY_PreferMin);
    if (tuple)
      prv_parse_tuple_int16(tuple, &settings.minimum_mg);
    
    tuple = dict_find(iter, MESSAGE_KEY_PreferMax);
    if (tuple)
      prv_parse_tuple_int16(tuple, &settings.maximum_mg);
    
  #endif
  
  prv_save_settings();
}
