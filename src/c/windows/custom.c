#include <pebble.h>
#include "src/c/windows/custom.h"
#include "src/c/modules/settings.h"
#include "src/c/modules/caffeine.h"
#include "src/c/helpers.h"

// --- PLATFORM SPECIFIC BEGIN ---
#if defined(PBL_PLATFORM_EMERY)
  #define SCREEN_W           200
  #define SCREEN_H           228

  #define MARGIN_TOP         4

  #define FONT_TOP_LABEL     FONT_KEY_GOTHIC_28_BOLD
  
  #define LABEL_H            36
  
#elif defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
  #define SCREEN_W           144
  #define SCREEN_H           168

  #define MARGIN_TOP         3

  #define FONT_TOP_LABEL     FONT_KEY_GOTHIC_28_BOLD
  
  #define LABEL_H            36

#elif defined(PBL_PLATFORM_CHALK)
  #error "Not implemented."
#else
  #error "Invalid target."
#endif
// --- PLATFORM SPECIFIC END ---

#define CONTENT_W          (SCREEN_W - ACTION_BAR_WIDTH)

#define FONT_CAFF_VALUE    FONT_KEY_GOTHIC_28_BOLD
#define VALUE_H            36

#define CAFF_VAL_Y         ((SCREEN_H / 2) - (VALUE_H / 2))

// Could display up to 5 digits, but that's crazy
#define MAX_CAFF           9999

static Window *s_custom_window;
static ActionBarLayer *s_action_bar;
static TextLayer *s_caffeine_text_layer;

static char s_caffeine_buf[8];
static int16_t s_caffeine;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(s_caffeine, settings.drink_time[1]);
  prv_post_caff_added();
}

static void select_short_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(s_caffeine, settings.drink_time[0]);
  prv_post_caff_added();
}

static void select_long_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(s_caffeine, settings.drink_time[2]);
  prv_post_caff_added();
}

static void refresh_caffeine() {
  snprintf(s_caffeine_buf, sizeof(s_caffeine_buf), "%dmg", s_caffeine);
  
  #ifdef PBL_COLOR
  // set text color based on limtis
  #endif
  
  layer_mark_dirty(text_layer_get_layer(s_caffeine_text_layer));
};

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_caffeine <= MAX_CAFF - settings.custom_step) {
    s_caffeine += settings.custom_step;
    refresh_caffeine();
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_caffeine >= settings.custom_step) {
    s_caffeine -= settings.custom_step;
    refresh_caffeine();
  }
}

static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 120, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 120, down_click_handler);
  
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 0, true, select_short_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_handler, NULL);
}

static void custom_window_load(Window *window) {
  s_caffeine = settings.custom_caff;
  
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  Layer *window_layer = window_get_root_layer(window);

  s_caffeine_text_layer = text_layer_create(GRect(0, CAFF_VAL_Y, CONTENT_W, VALUE_H));
  text_layer_set_text_alignment(s_caffeine_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_caffeine_text_layer, fonts_get_system_font(FONT_CAFF_VALUE));
  text_layer_set_text(s_caffeine_text_layer, s_caffeine_buf);
  
  refresh_caffeine();

  layer_add_child(window_layer, text_layer_get_layer(s_caffeine_text_layer));
}

static void custom_window_unload(Window *window) {
  action_bar_layer_destroy(s_action_bar);
  text_layer_destroy(s_caffeine_text_layer);
  
  window_destroy(s_custom_window);
}

void push_window_custom() {
  s_custom_window = window_create();
  window_set_click_config_provider(s_custom_window, click_config_provider);
  window_set_window_handlers(s_custom_window, (WindowHandlers) {
    .load = custom_window_load,
    .unload = custom_window_unload,
  });
  window_stack_push(s_custom_window, true);
}
