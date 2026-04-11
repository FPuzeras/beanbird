#pragma once
#include <pebble.h>
#include <stdint.h>

typedef struct {
  uint16_t pending_mg;
  uint16_t gut_mg;
  uint16_t blood_mg;
} caffeine_totals_t;

void caffeine_init();

void add_drink(int16_t miligrams, int8_t ingestion_duration);

caffeine_totals_t get_caffeine_totals();
