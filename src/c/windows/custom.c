#include <pebble.h>
#include "src/c/windows/custom.h"
#include "src/c/modules/settings.h"
#include "src/c/helpers.h"

static Window *s_custom_window;
static ActionBarLayer *s_action_bar;
static TextLayer *s_center_text_layer;
static int16_t caffeine;


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
  // add caffeine default
}

static void select_double_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_double_pulse();
  // add caffeine fast
}

static void select_long_down_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
}

static void select_long_up_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_long_pulse();
  // add caffeine slow
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  caffeine += settings.custom_step;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  caffeine -= settings.custom_step;
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 2, 0, true, select_double_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_down_handler, select_long_up_handler);
}

static void custom_window_load(Window *window) {
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_center_text_layer = text_layer_create(GRect(0, CENTER_TEXT_OFFSET_H, bounds.size.w - ACTION_BAR_WIDTH, CENTER_TEXT_SIZE_H));
  text_layer_set_text_alignment(s_center_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_center_text_layer, fonts_get_system_font(CENTER_TEXT_FONT));
  text_layer_set_text(s_center_text_layer, "0000");

  layer_add_child(window_layer, text_layer_get_layer(s_center_text_layer));
}

static void custom_window_unload(Window *window) {
  action_bar_layer_destroy(s_action_bar);
  text_layer_destroy(s_center_text_layer);
  
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
