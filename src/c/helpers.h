#pragma once
#include <pebble.h>
#include <stdint.h>
#include "src/c/modules/caffeine.h"

#define STORAGE_KEY_SETTINGS        1
#define STORAGE_KEY_DRINKS_BUF      2
#define STORAGE_KEY_DRINKS_META     3
#define STORAGE_KEY_CAFFEINE_STATS  4

static inline void prv_post_caff_added() {
  calculate_caffeine_stats();
  exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
  window_stack_pop_all(false);
}
