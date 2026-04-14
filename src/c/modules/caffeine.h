#pragma once
#include <pebble.h>
#include <stdint.h>

typedef struct {
  uint16_t pending_mg;
  uint16_t gut_mg;
  uint16_t blood_mg;
} caffeine_totals_t;

typedef struct {
  time_t sleep_time;
  time_t peak_time;
  uint16_t peak_mg;
} caffeine_stats_t;

void caffeine_init();

void add_drink(int16_t miligrams, int8_t ingestion_duration);

void metabolism_update_settings(int16_t hl_elim_m, int8_t hl_abs_m);

void update_sleep();

caffeine_totals_t get_caffeine_totals();

caffeine_stats_t get_caffeine_stats();

void caffeine_cleanup();
