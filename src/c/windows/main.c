#include <pebble.h>
#include "src/c/windows/main.h"
#include "src/c/windows/group.h"
#include "src/c/windows/custom.h"
#include "src/c/modules/settings.h"
#include "src/c/modules/caffeine.h"
#include "src/c/helpers.h"

static Window *s_main_window;
static ActionBarLayer *s_action_bar;

static TextLayer *s_center_text_layer;
static TextLayer *s_tr_text_layer;

static char s_center_text_buf[5];
static char s_tr_text_buf[5];

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  push_window_custom();
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

static void update_caffeine_text() {
  uint16_t b_mg = get_blood_caffeine();
  uint16_t g_mg = get_gut_caffeine();
  
  snprintf(s_center_text_buf, 5, "%04u", (unsigned)b_mg);
  snprintf(s_tr_text_buf, 5, "%04u", (unsigned)g_mg);

  text_layer_set_text(s_center_text_layer, s_center_text_buf);
  text_layer_set_text(s_tr_text_layer, s_tr_text_buf);
}

static void main_window_load(Window *window) {
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_center_text_layer = text_layer_create(GRect(0, CENTER_TEXT_OFFSET_H, bounds.size.w - ACTION_BAR_WIDTH, CENTER_TEXT_SIZE_H));
  text_layer_set_text_alignment(s_center_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_center_text_layer, fonts_get_system_font(CENTER_TEXT_FONT));
  
  s_tr_text_layer = text_layer_create(GRect(0, 0, bounds.size.w - ACTION_BAR_WIDTH, CENTER_TEXT_SIZE_H));
  text_layer_set_text_alignment(s_tr_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_tr_text_layer, fonts_get_system_font(CENTER_TEXT_FONT));
  
  update_caffeine_text();

  layer_add_child(window_layer, text_layer_get_layer(s_center_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_tr_text_layer));
}

static void main_window_unload(Window *window) {
  action_bar_layer_destroy(s_action_bar);
  
  text_layer_destroy(s_center_text_layer);
  text_layer_destroy(s_tr_text_layer);
  
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
