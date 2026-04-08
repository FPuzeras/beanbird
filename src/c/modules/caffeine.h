#pragma once
#include <pebble.h>
#include <stdint.h>

void caffeine_init();
void add_drink(int16_t miligrams, int16_t ingestion_duration);

uint16_t get_blood_caffeine();

uint16_t get_gut_caffeine();

time_t get_curfew();