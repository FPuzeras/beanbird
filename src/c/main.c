#include <pebble.h>
#include "src/c/settings.h"
#include "src/c/window_main.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  parse_inbox_settings(iter);
}

static void init(void) {
  load_settings();
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 128);
  push_window_main();
}

static void deinit(void) {
  window_stack_pop_all(true);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
