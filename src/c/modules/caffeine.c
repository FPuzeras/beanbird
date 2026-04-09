#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "src/c/helpers.h"
#include "src/c/modules/caffeine.h"
#include "src/c/modules/settings.h"

#define BUFFER_SIZE 32
#define CONTRIBUTION_CUTOFF 0.5f
#define EXPIRE_MAX_AGE (2 * SECONDS_PER_DAY)
#define CACHE_SECONDS 2

#define ke_s settings.elimination_constant
#define ka_s settings.absorbtion_constant

typedef struct {
  time_t time;
  int16_t dose;
  int16_t duration;
} ingestion;

typedef struct {
    int head;
    int count;
} BufferMetadata;

static ingestion ingestion_buffer[BUFFER_SIZE];
static BufferMetadata meta = { .head = -1, .count = 0 };

time_t last_value = 0;
uint16_t blood_caff = 0;
uint16_t gut_caff = 0;
uint16_t drink_caff = 0;

static void prv_save_to_storage() {
    persist_write_data(STORAGE_KEY_INGESTION, ingestion_buffer, sizeof(ingestion_buffer));
    persist_write_data(STORAGE_KEY_INGESTION_META, &meta, sizeof(meta));
}

void prv_ingestion_init() {
    if (persist_exists(STORAGE_KEY_INGESTION)) {
        persist_read_data(STORAGE_KEY_INGESTION, ingestion_buffer, sizeof(ingestion_buffer));
        persist_read_data(STORAGE_KEY_INGESTION_META, &meta, sizeof(meta));
    }
}

void prv_ingestion_push(ingestion item) {
    meta.head = (meta.head + 1) % BUFFER_SIZE;
    ingestion_buffer[meta.head] = item;
    
    if (meta.count < BUFFER_SIZE) {
        meta.count++;
    }
    
    prv_save_to_storage();
}

ingestion* prv_ingestion_head() {
    return (meta.count == 0) ? NULL : &ingestion_buffer[meta.head];
}

ingestion* prv_ingestion_next(ingestion* current) {
    if (meta.count <= 1 || current == NULL) return NULL;

    int current_idx = (int)(current - ingestion_buffer);
    int tail_idx = (meta.head - meta.count + 1 + BUFFER_SIZE) % BUFFER_SIZE;

    if (current_idx == tail_idx) return NULL;

    return &ingestion_buffer[(current_idx - 1 + BUFFER_SIZE) % BUFFER_SIZE];
}

bool prv_ingestion_remove(ingestion* target) {
    if (meta.count == 0 || target == NULL) return false;

    int target_idx = (int)(target - ingestion_buffer);
    int current = target_idx;
    int items_to_move;
    
    if (target_idx <= meta.head) {
        items_to_move = meta.head - target_idx;
    } else {
        items_to_move = (BUFFER_SIZE - target_idx) + meta.head;
    }

    for (int i = 0; i < items_to_move; i++) {
        int next_idx = (current + 1) % BUFFER_SIZE;
        ingestion_buffer[current] = ingestion_buffer[next_idx];
        current = next_idx;
    }

    meta.head = (meta.head - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    meta.count--;
    
    prv_save_to_storage();
    return true;
}

void caffeine_init() {
  prv_ingestion_init();
}

void compute_caffeine() {
  time_t current_time = time(NULL);
  
  if (current_time < last_value + CACHE_SECONDS)
    return;

  const float k_diff = ka_s - ke_s;
  const float inv_diff = (fabsf(k_diff) < 1e-7f) ? 1000000.0f : (1.0f / k_diff);
  const float inv_ka = 1.0f / ka_s;
  
  float total_blood_mg = 0.0f;
  float total_gut_mg = 0.0f;
  float total_drink_mg = 0.0f;
  
  ingestion* i = prv_ingestion_head();
  
  while (i != NULL) {
    // capture next pointer so we can remove current
    ingestion* next_drink = prv_ingestion_next(i);
    
    if (current_time - i->time > EXPIRE_MAX_AGE) {
        prv_ingestion_remove(i);
        i = next_drink;
        continue;
    }
    
    float t_elapsed = (float)(current_time - i->time);
    float d_sec = (float)(i->duration * SECONDS_PER_MINUTE);
    float dose = (float)i->dose;
    float g_mg = 0.0f;
    float b_mg = 0.0f;
    
    if (t_elapsed <= d_sec) {
      float R = dose / d_sec;
      float exp_ke = expf(-ke_s * t_elapsed);
      float exp_ka = expf(-ka_s * t_elapsed);

      float term1 = (1.0f - exp_ke) / ke_s;
      float term2 = (exp_ka - exp_ke) * inv_diff;
      
      g_mg = (R * inv_ka) * (1.0f - exp_ka);
      b_mg = R * (term1 + term2);
    } else {
      float R = dose / d_sec;
      float exp_ke_d = expf(-ke_s * d_sec);
      float exp_ka_d = expf(-ka_s * d_sec);

      float g_at_stop = (R * inv_ka) * (1.0f - exp_ka_d);
      float b_at_stop = R * (((1.0f - exp_ke_d) / ke_s) + ((exp_ka_d - exp_ke_d) * inv_diff));
      
      float t_post = t_elapsed - d_sec;
      float exp_ke_p = expf(-ke_s * t_post);
      float exp_ka_p = expf(-ka_s * t_post);

      g_mg = g_at_stop * exp_ka_p;
      b_mg = (b_at_stop * exp_ke_p) + (ka_s * g_at_stop * inv_diff) * (exp_ke_p - exp_ka_p);
    }
    
    if (t_elapsed > d_sec && b_mg < CONTRIBUTION_CUTOFF) {
        prv_ingestion_remove(i);
    } else {
        total_blood_mg += b_mg;
        total_gut_mg += g_mg;
        total_drink_mg += (dose - b_mg - g_mg);
    }
    
    i = next_drink;
  }
  
  last_value = current_time;
  blood_caff = float_to_u16(total_blood_mg);
  gut_caff = float_to_u16(total_gut_mg);
  drink_caff = float_to_u16(total_drink_mg);
}

uint16_t get_blood_caffeine() {
  compute_caffeine();
    
  return blood_caff;
}

uint16_t get_gut_caffeine() {
  compute_caffeine();
  
  return gut_caff;
}

uint16_t get_pending_caffeine() {
  compute_caffeine();
  
  return drink_caff;
}

void add_drink(int16_t miligrams, int16_t ingestion_duration) {
  ingestion i = {
    .dose = miligrams,
    .duration = ingestion_duration,
    .time = time(NULL)
  };
  prv_ingestion_push(i);
}
