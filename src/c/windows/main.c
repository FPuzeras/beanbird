#include <pebble.h>
#include "src/c/windows/main.h"
#include "src/c/windows/group.h"
#include "src/c/windows/custom.h"
#include "src/c/modules/settings.h"
#include "src/c/modules/caffeine.h"
#include "src/c/helpers.h"

// --- PLATFORM SPECIFIC BEGIN ---
#if defined(PBL_PLATFORM_EMERY)
  #define SCREEN_W           200
  #define SCREEN_H           228
  
  #define MARGIN_TOP         4
  
  #define LABEL_H            24
  #define VALUE_H            36
  #define SMALL_LABEL_H      18
  #define SMALL_VALUE_H      30
  
  #define FONT_TOP_LABEL     FONT_KEY_GOTHIC_18_BOLD
  #define FONT_TOP_VALUE     FONT_KEY_GOTHIC_28_BOLD
  #define FONT_MID_LABEL     FONT_KEY_GOTHIC_18
  #define FONT_MID_VALUE     FONT_KEY_GOTHIC_28_BOLD
  #define FONT_BOT_LABEL     FONT_KEY_GOTHIC_18_BOLD
  #define FONT_BOT_VALUE     FONT_KEY_GOTHIC_28_BOLD
  #define FONT_BOT_SUB       FONT_KEY_GOTHIC_14

  #define GUT_LABEL_TEXT     "GUT CAFF."
  #define DRINK_LABEL_TEXT   "DRINK CAFF."
  
#elif defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
  #define SCREEN_W           144
  #define SCREEN_H           168
  
  #define MARGIN_TOP         3
  
  #define LABEL_H            16
  #define VALUE_H            30
  #define SMALL_LABEL_H      16
  #define SMALL_VALUE_H      20
  
  #define FONT_TOP_LABEL     FONT_KEY_GOTHIC_14_BOLD
  #define FONT_TOP_VALUE     FONT_KEY_GOTHIC_24_BOLD
  #define FONT_MID_LABEL     FONT_KEY_GOTHIC_14
  #define FONT_MID_VALUE     FONT_KEY_GOTHIC_24_BOLD
  #define FONT_BOT_LABEL     FONT_KEY_GOTHIC_14_BOLD
  #define FONT_BOT_VALUE     FONT_KEY_GOTHIC_18_BOLD
  #define FONT_BOT_SUB       FONT_KEY_GOTHIC_14

  #define GUT_LABEL_TEXT     "GUT"
  #define DRINK_LABEL_TEXT   "DRINK"
  
#elif defined(PBL_PLATFORM_CHALK)
  #error "Not implemented."
#else
  #error "Invalid target."
#endif
// --- PLATFORM SPECIFIC END ---

#define CONTENT_W          (SCREEN_W - ACTION_BAR_WIDTH)
#define HALF_CONTENT_W     (CONTENT_W / 2)
#define SECTION_H          (SCREEN_H / 3)

#define TOP_VAL_Y          (MARGIN_TOP + LABEL_H)
#define MID_SEC_Y          SECTION_H
#define MID_LABEL_Y        (MID_SEC_Y + MARGIN_TOP)
#define MID_VAL_Y          (MID_LABEL_Y + SMALL_LABEL_H)
#define BOT_SEC_Y          (SECTION_H * 2)
#define BOT_LABEL_Y        (BOT_SEC_Y + MARGIN_TOP)
#define BOT_VAL_Y          (BOT_LABEL_Y + SMALL_LABEL_H)
#define BOT_SUB_Y          (BOT_VAL_Y + SMALL_VALUE_H)

#define REFRESH_MS         1000


static Window *s_main_window;
static ActionBarLayer *s_action_bar;
static Layer *s_canvas_layer;

static TextLayer *s_caffeine_label_layer, *s_caffeine_value_layer;
static TextLayer *s_pending_drink_label, *s_pending_drink_value;
static TextLayer *s_pending_gut_label, *s_pending_gut_value;
static TextLayer *s_sleep_label, *s_timer_layer, *s_sleep_sub_label;

static char s_blood_mg_buf[8], s_gut_mg_buf[6], s_pending_mg_buf[6];
static AppTimer* s_refresh_timer;


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

static void update_caffeine_text(void *_) {
  caffeine_totals_t totals = get_caffeine_totals();
  
  snprintf(s_blood_mg_buf, sizeof(s_blood_mg_buf), "%dmg", totals.blood_mg);
  snprintf(s_gut_mg_buf, sizeof(s_gut_mg_buf), "%d", totals.gut_mg);
  snprintf(s_pending_mg_buf, sizeof(s_pending_mg_buf), "%d",  totals.pending_mg);

  text_layer_set_text(s_caffeine_value_layer, s_blood_mg_buf);
  text_layer_set_text(s_pending_gut_value, s_gut_mg_buf);
  text_layer_set_text(s_pending_drink_value, s_pending_mg_buf);
  
  #ifdef PBL_COLOR
  if (totals.blood_mg > settings.maximum_mg) {
    text_layer_set_text_color(s_caffeine_value_layer, GColorDarkCandyAppleRed);
  } else if (totals.blood_mg >= settings.minimum_mg) {
    text_layer_set_text_color(s_caffeine_value_layer, GColorBlack);
  } else {
    text_layer_set_text_color(s_caffeine_value_layer, GColorCadetBlue);
  }
  #endif
  
  s_refresh_timer = app_timer_register(REFRESH_MS, update_caffeine_text, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorLightGray);
  #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  
  graphics_context_set_stroke_width(ctx, 2);
  
  graphics_draw_line(ctx, GPoint(0, SECTION_H), GPoint(CONTENT_W, SECTION_H));
  graphics_draw_line(ctx, GPoint(0, SECTION_H * 2), GPoint(CONTENT_W, SECTION_H * 2));
  graphics_draw_line(ctx, GPoint(HALF_CONTENT_W, SECTION_H), GPoint(HALF_CONTENT_W, SECTION_H * 2));
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  s_canvas_layer = layer_create(GRect(0, 0, CONTENT_W, SCREEN_H));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  s_caffeine_label_layer = text_layer_create(GRect(0, MARGIN_TOP, CONTENT_W, LABEL_H));
  text_layer_set_text(s_caffeine_label_layer, "BLOOD CAFFEINE");
  text_layer_set_font(s_caffeine_label_layer, fonts_get_system_font(FONT_TOP_LABEL));
  text_layer_set_text_alignment(s_caffeine_label_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_caffeine_label_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_caffeine_label_layer));

  s_caffeine_value_layer = text_layer_create(GRect(0, TOP_VAL_Y, CONTENT_W, VALUE_H));
  text_layer_set_font(s_caffeine_value_layer, fonts_get_system_font(FONT_TOP_VALUE));
  text_layer_set_text_alignment(s_caffeine_value_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_caffeine_value_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_caffeine_value_layer));

  s_pending_drink_label = text_layer_create(GRect(2, MID_LABEL_Y, HALF_CONTENT_W - 4, SMALL_LABEL_H));
  text_layer_set_text(s_pending_drink_label, DRINK_LABEL_TEXT);
  text_layer_set_font(s_pending_drink_label, fonts_get_system_font(FONT_MID_LABEL));
  text_layer_set_text_alignment(s_pending_drink_label, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_pending_drink_label));

  s_pending_drink_value = text_layer_create(GRect(2, MID_VAL_Y, HALF_CONTENT_W - 4, VALUE_H));
  text_layer_set_font(s_pending_drink_value, fonts_get_system_font(FONT_MID_VALUE));
  text_layer_set_text_alignment(s_pending_drink_value, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_pending_drink_value));

  s_pending_gut_label = text_layer_create(GRect(HALF_CONTENT_W + 2, MID_LABEL_Y, HALF_CONTENT_W - 4, SMALL_LABEL_H));
  text_layer_set_text(s_pending_gut_label, GUT_LABEL_TEXT);
  text_layer_set_font(s_pending_gut_label, fonts_get_system_font(FONT_MID_LABEL));
  text_layer_set_text_alignment(s_pending_gut_label, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_pending_gut_label));

  s_pending_gut_value = text_layer_create(GRect(HALF_CONTENT_W + 2, MID_VAL_Y, HALF_CONTENT_W - 4, VALUE_H));
  text_layer_set_font(s_pending_gut_value, fonts_get_system_font(FONT_MID_VALUE));
  text_layer_set_text_alignment(s_pending_gut_value, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_pending_gut_value));

  s_sleep_label = text_layer_create(GRect(0, BOT_LABEL_Y, CONTENT_W, SMALL_LABEL_H));
  text_layer_set_text(s_sleep_label, "SLEEP READY:");
  text_layer_set_font(s_sleep_label, fonts_get_system_font(FONT_BOT_LABEL));
  text_layer_set_text_alignment(s_sleep_label, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_sleep_label));

  s_timer_layer = text_layer_create(GRect(0, BOT_VAL_Y, CONTENT_W, SMALL_VALUE_H));
  text_layer_set_text(s_timer_layer, "00:00:00");
  text_layer_set_font(s_timer_layer, fonts_get_system_font(FONT_BOT_VALUE));
  text_layer_set_text_alignment(s_timer_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_timer_layer));

  s_sleep_sub_label = text_layer_create(GRect(0, BOT_SUB_Y, CONTENT_W, SMALL_LABEL_H));
  text_layer_set_text(s_sleep_sub_label, "until <50mg");
  text_layer_set_font(s_sleep_sub_label, fonts_get_system_font(FONT_BOT_SUB));
  text_layer_set_text_alignment(s_sleep_sub_label, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_sleep_sub_label));
  
  update_caffeine_text(NULL);
}

static void main_window_unload(Window *window) {
  app_timer_cancel(s_refresh_timer);
  action_bar_layer_destroy(s_action_bar);
  layer_destroy(s_canvas_layer);
  
  text_layer_destroy(s_caffeine_label_layer);
  text_layer_destroy(s_caffeine_value_layer);
  text_layer_destroy(s_pending_drink_label);
  text_layer_destroy(s_pending_drink_value);
  text_layer_destroy(s_pending_gut_label);
  text_layer_destroy(s_pending_gut_value);
  text_layer_destroy(s_sleep_label);
  text_layer_destroy(s_timer_layer);
  text_layer_destroy(s_sleep_sub_label);
  
  window_destroy(s_main_window);
}

void refresh_stats() {
  // TODO
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
