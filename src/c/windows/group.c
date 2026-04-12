#include <pebble.h>
#include "src/c/windows/group.h"
#include "src/c/modules/caffeine.h"
#include "src/c/helpers.h"

#if defined(PBL_PLATFORM_EMERY)
  #define SCREEN_W           200
  #define SCREEN_H           228

  #define ACTION_COMPRESS    30
  
  #define MARGIN_SIDE        6

  #define LABEL_H            32
  #define FONT_LABEL         FONT_KEY_GOTHIC_28_BOLD
  
#elif defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_DIORITE) || defined(PBL_PLATFORM_FLINT)
  #define SCREEN_W           144
  #define SCREEN_H           168

  #define ACTION_COMPRESS    10
  
  #define MARGIN_SIDE        4
  
  #define LABEL_H            24
  #define FONT_LABEL         FONT_KEY_GOTHIC_24_BOLD
  
#elif defined(PBL_PLATFORM_CHALK)
  #error "Not implemented."
#else
  #error "Invalid target."
#endif
// --- PLATFORM SPECIFIC END ---

#define CONTENT_H          (SCREEN_H - ACTION_COMPRESS * 2)
#define CONTENT_W          (SCREEN_W - ACTION_BAR_WIDTH)
#define LABEL_W            (CONTENT_W - 2 * MARGIN_SIDE)
#define SECTION_H          (CONTENT_H / 3)
#define LABEL_OFFSET       ((SECTION_H / 2) - (LABEL_H / 2))

#define TOP_LABEL_Y        (LABEL_OFFSET + ACTION_COMPRESS)
#define MID_SEC_Y          SECTION_H + ACTION_COMPRESS
#define MID_LABEL_Y        (MID_SEC_Y + LABEL_OFFSET)
#define BOT_SEC_Y          (SECTION_H * 2 + ACTION_COMPRESS)
#define BOT_LABEL_Y        (BOT_SEC_Y + LABEL_OFFSET)

static Window *s_group_window;
static ActionBarLayer *s_action_bar;
static Layer *s_canvas_layer;

static TextLayer *s_text_top;
static TextLayer *s_text_select;
static TextLayer *s_text_bottom;

static GBitmap *s_icon_add;

static int16_t *drink_caff;
static char (*drink_titles)[STR_MAXLEN+1];

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[0], settings.drink_time[1]);
  prv_post_caff_added();
}
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[1], settings.drink_time[1]);
  prv_post_caff_added();
}
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[2], settings.drink_time[1]);
  prv_post_caff_added();
}


static void up_long_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[0], settings.drink_time[2]);
  prv_post_caff_added();
}
static void select_long_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[1], settings.drink_time[2]);
  prv_post_caff_added();
}
static void down_long_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[2], settings.drink_time[2]);
  prv_post_caff_added();
}


static void up_short_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[0], settings.drink_time[0]);
  prv_post_caff_added();
}
static void select_short_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[1], settings.drink_time[0]);
  prv_post_caff_added();
}
static void down_short_handler(ClickRecognizerRef recognizer, void *context) {
  add_drink(drink_caff[2], settings.drink_time[0]);
  prv_post_caff_added();
}


static void click_config_provider(void *context) {
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, 0, true, up_short_handler);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 0, true, select_short_handler);
  window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 0, 0, true, down_short_handler);
  
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  
  window_long_click_subscribe(BUTTON_ID_UP, 0, up_long_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, down_long_handler, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorLightGray);
  #else
    graphics_context_set_stroke_color(ctx, GColorBlack);
  #endif
  
  graphics_context_set_stroke_width(ctx, 2);
  
  graphics_draw_line(ctx, GPoint(0, SECTION_H + ACTION_COMPRESS), GPoint(CONTENT_W, SECTION_H + ACTION_COMPRESS));
  graphics_draw_line(ctx, GPoint(0, SECTION_H * 2 + ACTION_COMPRESS), GPoint(CONTENT_W, SECTION_H * 2 + ACTION_COMPRESS));
}

static void group_window_load(Window *window) {
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  Layer *window_layer = window_get_root_layer(window);
  
  s_icon_add = gbitmap_create_with_resource(RESOURCE_ID_ICON_ACTION_ADD);

  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_icon_add);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_icon_add);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_add);
  
  s_canvas_layer = layer_create(GRect(0, 0, CONTENT_W, SCREEN_H));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
  
  s_text_top = text_layer_create(GRect(MARGIN_SIDE, TOP_LABEL_Y, LABEL_W, LABEL_H));
  s_text_select = text_layer_create(GRect(MARGIN_SIDE, MID_LABEL_Y, LABEL_W, LABEL_H));
  s_text_bottom = text_layer_create(GRect(MARGIN_SIDE, BOT_LABEL_Y, LABEL_W, LABEL_H));
  
  text_layer_set_text(s_text_top, drink_titles[0]);
  text_layer_set_text(s_text_select, drink_titles[1]);
  text_layer_set_text(s_text_bottom, drink_titles[2]);

  text_layer_set_text_alignment(s_text_top, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_text_select, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_text_bottom, GTextAlignmentLeft);
  
  layer_add_child(window_layer, text_layer_get_layer(s_text_top));
  layer_add_child(window_layer, text_layer_get_layer(s_text_select));
  layer_add_child(window_layer, text_layer_get_layer(s_text_bottom));
}

static void group_window_unload(Window *window) {
  text_layer_destroy(s_text_top);
  text_layer_destroy(s_text_select);
  text_layer_destroy(s_text_bottom);
  
  action_bar_layer_destroy(s_action_bar);
  gbitmap_destroy(s_icon_add);
  
  layer_destroy(s_canvas_layer);
  
  window_destroy(s_group_window);
}

void push_window_group(int16_t *caff_content, char (*titles)[STR_MAXLEN + 1]) {
  drink_caff = caff_content;
  drink_titles = titles;
  
  s_group_window = window_create();
  window_set_click_config_provider(s_group_window, click_config_provider);
  window_set_window_handlers(s_group_window, (WindowHandlers) {
    .load = group_window_load,
    .unload = group_window_unload,
  });
  window_stack_push(s_group_window, true);
}
