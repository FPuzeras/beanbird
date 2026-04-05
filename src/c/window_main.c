#include <pebble.h>
#include "src/c/window_main.h"
#include "src/c/settings.h"

static Window *s_main_window;


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
//   text_layer_set_text(s_text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
//   text_layer_set_text(s_text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
//   text_layer_set_text(s_text_layer, "Down");
}

static void click_config_provider(void *context) {  
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
//   Layer *window_layer = window_get_root_layer(window);
//   GRect bounds = layer_get_bounds(window_layer);

//   s_text_layer = text_layer_create(GRect(0, 70, bounds.size.w, 20));
//   text_layer_set_text(s_text_layer, "Press a button");
//   text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
//   layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
//   text_layer_destroy(s_text_layer);
}

void push_window_main() {
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}
