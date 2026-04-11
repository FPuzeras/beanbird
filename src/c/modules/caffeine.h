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

caffeine_totals_t get_caffeine_totals();

void calculate_caffeine_stats();

caffeine_stats_t get_caffeine_stats();