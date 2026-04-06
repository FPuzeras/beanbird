#include <pebble.h>
#include "src/c/window_group.h"

static Window *s_group_window;
static ActionBarLayer *s_action_bar;

static TextLayer *s_text_top;
static TextLayer *s_text_select;
static TextLayer *s_text_bottom;

static GBitmap *s_up_icon;

static int16_t *drink_caff;
static char (*drink_titles)[STR_MAXLEN+1];

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//   text_layer_set_text(s_text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
//   text_layer_set_text(s_text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
//   text_layer_set_text(s_text_layer, "Down");
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
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_text_top = text_layer_create(GRect(0, 30, bounds.size.w - ACTION_BAR_WIDTH, 20));
  s_text_select = text_layer_create(GRect(0, 70, bounds.size.w - ACTION_BAR_WIDTH, 20));
  s_text_bottom = text_layer_create(GRect(0, 110, bounds.size.w - ACTION_BAR_WIDTH, 20));
  
  text_layer_set_text(s_text_top, drink_titles[0]);
  text_layer_set_text(s_text_select, drink_titles[1]);
  text_layer_set_text(s_text_bottom, drink_titles[2]);

  
  text_layer_set_text_alignment(s_text_top, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_text_select, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_text_bottom, GTextAlignmentLeft);
  
  layer_add_child(window_layer, text_layer_get_layer(s_text_top));
  layer_add_child(window_layer, text_layer_get_layer(s_text_select));
  layer_add_child(window_layer, text_layer_get_layer(s_text_bottom));
  
  
//   s_up_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
}

static void main_window_unload(Window *window) {
  
//   gbitmap_destroy(s_up_icon);
}

void push_window_group(int16_t *caff_content, char (*titles)[STR_MAXLEN + 1]) {
  drink_caff = caff_content;
  drink_titles = titles;
  
  s_group_window = window_create();
  window_set_click_config_provider(s_group_window, click_config_provider);
  window_set_window_handlers(s_group_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_group_window, true);
}
