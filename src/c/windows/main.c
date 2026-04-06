#include <pebble.h>
#include "src/c/windows/main.h"
#include "src/c/windows/group.h"
#include "src/c/modules/settings.h"

static Window *s_main_window;
static ActionBarLayer *s_action_bar;


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
//   text_layer_set_text(s_text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  push_window_group(&settings.caff_content[0], &settings.drink_titles[0]);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    push_window_group(&settings.caff_content[3], &settings.drink_titles[3]);
}

static void click_config_provider(void *context) {  
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
//   Layer *window_layer = window_get_root_layer(window);
//   GRect bounds = layer_get_bounds(window_layer);

//   s_text_layer = text_layer_create(GRect(0, 70, bounds.size.w, 20));
//   text_layer_set_text(s_text_layer, "Press a button");
//   text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
//   layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
  action_bar_layer_destroy(s_action_bar);
//   text_layer_destroy(s_text_layer);
  
    window_destroy(s_main_window);
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
