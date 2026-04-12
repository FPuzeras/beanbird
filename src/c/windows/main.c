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
  #define SIDE_PADDING       6
  
  #define LABEL_H            24
  #define VALUE_H            36
  #define SMALL_LABEL_H      18
  #define SMALL_VALUE_H      30
  
  #define FONT_TOP_LABEL     FONT_KEY_GOTHIC_18_BOLD
  #define FONT_TOP_VALUE     FONT_KEY_GOTHIC_28_BOLD
  #define FONT_MID_LABEL     FONT_KEY_GOTHIC_18
  #define FONT_MID_VALUE     FONT_KEY_GOTHIC_28_BOLD
  #define FONT_BOT_FIELD     FONT_KEY_GOTHIC_18_BOLD

  #define GUT_LABEL_TEXT     "GUT CAFF."
  #define DRINK_LABEL_TEXT   "DRINK CAFF."
  
#elif defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
  #define SCREEN_W           144
  #define SCREEN_H           168
  
  #define MARGIN_TOP         3
  #define SIDE_PADDING       4
  
  #define LABEL_H            16
  #define VALUE_H            30
  #define SMALL_LABEL_H      16
  #define SMALL_VALUE_H      20
  
  #define FONT_TOP_LABEL     FONT_KEY_GOTHIC_14_BOLD
  #define FONT_TOP_VALUE     FONT_KEY_GOTHIC_24_BOLD
  #define FONT_MID_LABEL     FONT_KEY_GOTHIC_14
  #define FONT_MID_VALUE     FONT_KEY_GOTHIC_24_BOLD
  #define FONT_BOT_FIELD     FONT_KEY_GOTHIC_14

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
#define HALF_SECTION_H     (SECTION_H / 2)

#define TOP_VAL_Y          (MARGIN_TOP + LABEL_H)
#define MID_SEC_Y          SECTION_H
#define MID_LABEL_Y        (MID_SEC_Y + MARGIN_TOP)
#define MID_VAL_Y          (MID_LABEL_Y + SMALL_LABEL_H)
#define BOT_SEC_Y          (SECTION_H * 2)
#define BOT_FIELD_1_Y      (BOT_SEC_Y + MARGIN_TOP)
#define BOT_FIELD_2_Y      (BOT_FIELD_1_Y + HALF_SECTION_H + MARGIN_TOP)

#define REFRESH_MS         1000


static Window *s_main_window;
static ActionBarLayer *s_action_bar;
static Layer *s_canvas_layer;

static TextLayer *s_caffeine_label_layer, *s_caffeine_value_layer;
static TextLayer *s_pending_drink_label, *s_pending_drink_value;
static TextLayer *s_pending_gut_label, *s_pending_gut_value;
static TextLayer *s_sleep_field, *s_peak_field;

static char s_blood_mg_buf[8], s_gut_mg_buf[6], s_pending_mg_buf[6];
static char s_sleep_buf[20], s_peak_buf[20];
static AppTimer* s_refresh_timer;

static AppTimer* s_refresh_timer;

static GBitmap *s_icon_up;
static GBitmap *s_icon_more;
static GBitmap *s_icon_down;

static GBitmap *s_icon_sleep;
static GBitmap *s_icon_peak;

static BitmapLayer *s_sleep_layer;
static BitmapLayer *s_peak_layer;

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
  
  caffeine_stats_t caff_stats = get_caffeine_stats();
  time_t current_time = time(NULL);
  
  int sleep_time = current_time > caff_stats.sleep_time ? 0 : caff_stats.sleep_time - current_time ;
  
  snprintf(s_sleep_buf, sizeof(s_sleep_buf), "<%umg %02d:%02d:%02d", 
           settings.sleep_mg,
           sleep_time / SECONDS_PER_HOUR, // HOLY SHIT THIS FUCKING LINE
           (sleep_time % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE,
           sleep_time % SECONDS_PER_MINUTE);
  
  int peak_time = current_time > caff_stats.peak_time ? 0 : caff_stats.peak_time - current_time;
  snprintf(s_peak_buf, sizeof(s_peak_buf), "%umg %02d:%02d:%02d", 
           caff_stats.peak_mg,
           peak_time / SECONDS_PER_HOUR, // HOLY SHIT THIS FUCKING LINE
           (peak_time % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE,
           peak_time % SECONDS_PER_MINUTE);
  
  layer_mark_dirty(text_layer_get_layer(s_sleep_field));
  layer_mark_dirty(text_layer_get_layer(s_peak_field));
  
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
  graphics_draw_line(ctx, GPoint(0, SECTION_H * 2 + HALF_SECTION_H), GPoint(CONTENT_W, SECTION_H * 2 + HALF_SECTION_H));
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
  
  s_icon_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_ACTION_UP);
  s_icon_more = gbitmap_create_with_resource(RESOURCE_ID_ICON_ACTION_MORE);
  s_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_ACTION_DOWN);
  
  s_icon_sleep = gbitmap_create_with_resource(RESOURCE_ID_ICON_SLEEP);
  s_icon_peak = gbitmap_create_with_resource(RESOURCE_ID_ICON_PEAK);
  
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_icon_up);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_icon_more);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_down);

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

  s_sleep_field = text_layer_create(GRect(0, BOT_FIELD_1_Y + 1, CONTENT_W - SIDE_PADDING, LABEL_H));
  text_layer_set_text(s_sleep_field, s_sleep_buf);
  text_layer_set_font(s_sleep_field, fonts_get_system_font(FONT_BOT_FIELD));
  text_layer_set_text_alignment(s_sleep_field, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_sleep_field));

  s_peak_field = text_layer_create(GRect(0, BOT_FIELD_2_Y, CONTENT_W - SIDE_PADDING, LABEL_H));
  text_layer_set_text(s_peak_field, s_peak_buf);
  text_layer_set_font(s_peak_field, fonts_get_system_font(FONT_BOT_FIELD));
  text_layer_set_text_alignment(s_peak_field, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_peak_field));
  
  s_sleep_layer = bitmap_layer_create(GRect(SIDE_PADDING, BOT_FIELD_1_Y + 1, 25, LABEL_H));
  bitmap_layer_set_bitmap(s_sleep_layer, s_icon_sleep);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_sleep_layer));
  
  s_peak_layer = bitmap_layer_create(GRect(SIDE_PADDING, BOT_FIELD_2_Y + 1, 25, LABEL_H));
  bitmap_layer_set_bitmap(s_peak_layer, s_icon_peak);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_peak_layer));
  
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
  text_layer_destroy(s_sleep_field);
  text_layer_destroy(s_peak_field);
  
  bitmap_layer_destroy(s_sleep_layer);
  bitmap_layer_destroy(s_peak_layer);
  
  gbitmap_destroy(s_icon_up);
  gbitmap_destroy(s_icon_more);
  gbitmap_destroy(s_icon_down);
  gbitmap_destroy(s_icon_sleep);
  gbitmap_destroy(s_icon_peak);
  
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
