#include <pebble.h>
#include "src/c/windows/main.h"
#include "src/c/windows/group.h"
#include "src/c/windows/custom.h"
#include "src/c/modules/settings.h"
#include "src/c/modules/caffeine.h"
#include "src/c/helpers.h"

static Window *s_main_window;
static ActionBarLayer *s_action_bar;
static Layer *s_canvas_layer;

// top section
static TextLayer *s_caffeine_label_layer;
static TextLayer *s_caffeine_value_layer;
static BitmapLayer *s_logo_layer;
static GBitmap *s_logo_bitmap;

// middle section
static TextLayer *s_pending_drink_label;
static TextLayer *s_pending_drink_value;

static TextLayer *s_pending_gut_label;
static TextLayer *s_pending_gut_value;

// bottom section
static TextLayer *s_sleep_label;
static TextLayer *s_timer_layer;
static TextLayer *s_sleep_sub_label;
static BitmapLayer *s_zzz_icon_layer;
static GBitmap *s_zzz_icon_bitmap;

static AppTimer* s_refresh_timer;

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

// static void update_caffeine_text(void *_) {
//   uint16_t b_mg = get_blood_caffeine();
//   uint16_t g_mg = get_gut_caffeine();
//   uint16_t p_mg = get_pending_caffeine();

//   snprintf(s_center_text_buf, 5, "%04u", (unsigned)b_mg);
//   snprintf(s_tr_text_buf, 5, "%04u", (unsigned)g_mg);
//   snprintf(s_tl_text_buf, 5, "%04u", (unsigned)p_mg);


//   text_layer_set_text(s_center_text_layer, s_center_text_buf);
//   text_layer_set_text(s_tr_text_layer, s_tr_text_buf);
//   text_layer_set_text(s_tl_text_layer, s_tl_text_buf);

  
//   s_refresh_timer = app_timer_register(2000, update_caffeine_text, NULL);
// }

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w - ACTION_BAR_WIDTH;
  int h = bounds.size.h;
  int third_h = h / 3;
  int half_w = w / 2;

  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorLightGray);
  #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  
  graphics_context_set_stroke_width(ctx, 2);
  
  graphics_draw_line(ctx, GPoint(0, third_h), GPoint(w, third_h));
  graphics_draw_line(ctx, GPoint(0, third_h * 2), GPoint(w, third_h * 2));
  graphics_draw_line(ctx, GPoint(half_w, third_h), GPoint(half_w, third_h * 2));
}

static void main_window_load(Window *window) {
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
  
  int w = bounds.size.w - ACTION_BAR_WIDTH;
  int h = bounds.size.h;
  int third_h = h / 3;
  int half_w = w / 2;
  
  #ifdef PBL_ROUND
    int margin_y = 15;
  #else
    int margin_y = 5;
  #endif
  
//   s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO);
//   s_logo_layer = bitmap_layer_create(GRect(margin_x, margin_y, 20, 20));
//   bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
//   layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_layer));

  s_caffeine_label_layer = text_layer_create(GRect(0, margin_y, w, 20));
  text_layer_set_text(s_caffeine_label_layer, "Blood Caffeine");
  text_layer_set_font(s_caffeine_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_caffeine_label_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_caffeine_label_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_caffeine_label_layer));

  s_caffeine_value_layer = text_layer_create(GRect(0, margin_y + 20, w, 36));
  text_layer_set_text(s_caffeine_value_layer, "125 mg");
  text_layer_set_font(s_caffeine_value_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_caffeine_value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_caffeine_value_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_caffeine_value_layer, GColorRed);
  #endif
  layer_add_child(window_layer, text_layer_get_layer(s_caffeine_value_layer));

  s_pending_drink_label = text_layer_create(GRect(0, third_h + 4, half_w, 18));
  text_layer_set_text(s_pending_drink_label, "Pend. Drink:");
  text_layer_set_font(s_pending_drink_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_pending_drink_label, GTextAlignmentCenter);
  text_layer_set_background_color(s_pending_drink_label, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_pending_drink_label));

  s_pending_drink_value = text_layer_create(GRect(0, third_h + 20, half_w, 28));
  text_layer_set_text(s_pending_drink_value, "40");
  text_layer_set_font(s_pending_drink_value, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_pending_drink_value, GTextAlignmentCenter);
  text_layer_set_background_color(s_pending_drink_value, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_pending_drink_value, GColorBlue);
  #endif
  layer_add_child(window_layer, text_layer_get_layer(s_pending_drink_value));

  s_pending_gut_label = text_layer_create(GRect(half_w, third_h + 4, half_w, 18));
  text_layer_set_text(s_pending_gut_label, "Pend. Gut:");
  text_layer_set_font(s_pending_gut_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_pending_gut_label, GTextAlignmentCenter);
  text_layer_set_background_color(s_pending_gut_label, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_pending_gut_label));

  s_pending_gut_value = text_layer_create(GRect(half_w, third_h + 20, half_w, 28));
  text_layer_set_text(s_pending_gut_value, "15");
  text_layer_set_font(s_pending_gut_value, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_pending_gut_value, GTextAlignmentCenter);
  text_layer_set_background_color(s_pending_gut_value, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_pending_gut_value, GColorGreen);
  #endif
  layer_add_child(window_layer, text_layer_get_layer(s_pending_gut_value));


  int bot_y = third_h * 2;

  s_sleep_label = text_layer_create(GRect(0, bot_y + 2, w, 18));
  text_layer_set_text(s_sleep_label, "Sleep Ready:");
  text_layer_set_font(s_sleep_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_sleep_label, GTextAlignmentCenter);
  text_layer_set_background_color(s_sleep_label, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_sleep_label));

  s_timer_layer = text_layer_create(GRect(0, bot_y + 20, w, 28));
  text_layer_set_text(s_timer_layer, "04:30:00");
  text_layer_set_font(s_timer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_timer_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_timer_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_timer_layer, GColorPurple);
  #endif
  layer_add_child(window_layer, text_layer_get_layer(s_timer_layer));

  s_sleep_sub_label = text_layer_create(GRect(0, bot_y + 48, w, 18));
  text_layer_set_text(s_sleep_sub_label, "until < 50mg");
  text_layer_set_font(s_sleep_sub_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_sleep_sub_label, GTextAlignmentCenter);
  text_layer_set_background_color(s_sleep_sub_label, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_sleep_sub_label));

//   s_zzz_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ZZZ_ICON);
//   #ifdef PBL_ROUND
//     s_zzz_icon_layer = bitmap_layer_create(GRect((w / 2) + 45, bot_y + 24, 20, 20));
//   #else
//     s_zzz_icon_layer = bitmap_layer_create(GRect(w - 30, bot_y + 24, 20, 20));
//   #endif
//   bitmap_layer_set_bitmap(s_zzz_icon_layer, s_zzz_icon_bitmap);
//   layer_add_child(window_layer, bitmap_layer_get_layer(s_zzz_icon_layer));
}

static void main_window_unload(Window *window) {
  app_timer_cancel(s_refresh_timer);
  action_bar_layer_destroy(s_action_bar);
  layer_destroy(s_canvas_layer);
  
  // top text & bitmaps
  text_layer_destroy(s_caffeine_label_layer);
  text_layer_destroy(s_caffeine_value_layer);
  bitmap_layer_destroy(s_logo_layer);
  gbitmap_destroy(s_logo_bitmap);

  // middle text & bitmaps
  text_layer_destroy(s_pending_drink_label);
  text_layer_destroy(s_pending_drink_value);

  text_layer_destroy(s_pending_gut_label);
  text_layer_destroy(s_pending_gut_value);

  // bottom text & bitmaps
  text_layer_destroy(s_sleep_label);
  text_layer_destroy(s_timer_layer);
  text_layer_destroy(s_sleep_sub_label);
  bitmap_layer_destroy(s_zzz_icon_layer);
  gbitmap_destroy(s_zzz_icon_bitmap);
  
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
