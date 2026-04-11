#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "src/c/helpers.h"
#include "src/c/modules/caffeine.h"
#include "src/c/modules/settings.h"

#define BUFFER_SIZE 32
#define CONTRIBUTION_CUTOFF 0.5f
#define EXPIRE_MAX_AGE (2 * SECONDS_PER_DAY)
#define CACHE_SECONDS 1

#define ke_s settings.elimination_constant
#define ka_s settings.absorbtion_constant

typedef struct {
  time_t time;
  int16_t dose;
  int8_t duration;
} ingestion;

typedef struct {
    int head;
    int count;
} BufferMetadata;

static ingestion ingestion_buffer[BUFFER_SIZE];
static BufferMetadata meta = { .head = -1, .count = 0 };

time_t last_value = 0;

float caffeine_blood_mg = 0.0f;
float caffeine_gut_mg = 0.0f;
float caffeine_pending_mg = 0.0f;

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

void compute_caffeine(time_t current_time) {
  const float k_diff = ka_s - ke_s;
  const float inv_diff = (fabsf(k_diff) < 1e-7f) ? 1000000.0f : (1.0f / k_diff);
  const float inv_ka = 1.0f / ka_s;
  const float inv_ke = 1.0f / ke_s;
  
  caffeine_blood_mg = 0.0f;
  caffeine_gut_mg = 0.0f;
  caffeine_pending_mg = 0.0f;

  
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
    
    // If exp(-ke * t) results < 0.1mg for large (>600mg) dose, skip it
    float exp_ke_check = -ke_s * t_elapsed;
    if (exp_ke_check < -10.0f) {
        prv_ingestion_remove(i);
        i = next_drink;
        continue;
    }
    
    float d_sec = (float)i->duration * 60.0f;
    float dose = (float)i->dose;
    float R = dose / d_sec;
    float g_mg = 0.0f;
    float b_mg = 0.0f;
    float p_mg = 0.0f;
    
    if (t_elapsed <= d_sec) {
      // During Ingestion
      float p_ka = -ka_s * t_elapsed;
      float p_ke = -ke_s * t_elapsed;
      
      // clamp exponents for FPU safety
      if (p_ka < -20.0f) p_ka = -20.0f;
      if (p_ke < -20.0f) p_ke = -20.0f;

      float ex_ka = expf(p_ka);
      float ex_ke = expf(p_ke);
      
      g_mg = (R * inv_ka) * (1.0f - ex_ka);
      b_mg = R * (((1.0f - ex_ke) * inv_ke) + ((ex_ka - ex_ke) * inv_diff));
      p_mg = R * (d_sec - t_elapsed);
    } else {
      // Post-Ingestion
      float p_ka_d = -ka_s * d_sec;
      float p_ke_d = -ke_s * d_sec;
      
      // clamp exponents for FPU safety
      if (p_ka_d < -20.0f) p_ka_d = -20.0f;
      if (p_ke_d < -20.0f) p_ke_d = -20.0f;
      
      float ex_ka_d = expf(p_ka_d);
      float ex_ke_d = expf(p_ke_d);

      float g_at_stop = (R * inv_ka) * (1.0f - ex_ka_d);
      float b_at_stop = R * (((1.0f - ex_ke_d) * inv_ke) + ((ex_ka_d - ex_ke_d) * inv_diff));

      float t_post = t_elapsed - d_sec;
      float p_ka_p = -ka_s * t_post;
      float p_ke_p = -ke_s * t_post;
      // clamp exponents for FPU safety
      if (p_ka_p < -20.0f) p_ka_p = -20.0f;
      if (p_ke_p < -20.0f) p_ke_p = -20.0f;

      float ex_ka_p = expf(p_ka_p);
      float ex_ke_p = expf(p_ke_p);
      
      g_mg = g_at_stop * ex_ka_p;
      b_mg = (b_at_stop * ex_ke_p) + (ka_s * g_at_stop * inv_diff) * (ex_ke_p - ex_ka_p);
      p_mg = 0.0f;
    }
    
    if (t_elapsed > d_sec && b_mg < CONTRIBUTION_CUTOFF) {
        prv_ingestion_remove(i);
    } else {
        caffeine_blood_mg += b_mg;
        caffeine_gut_mg += g_mg;
        caffeine_pending_mg += p_mg;
    }
    
    i = next_drink;
  }
  
  last_value = current_time;
}

caffeine_totals_t get_caffeine_totals() {
  time_t current_time = time(NULL);
  
  if (current_time > last_value + CACHE_SECONDS)
    compute_caffeine(current_time);
  
  caffeine_totals_t totals = {
    .blood_mg = float_to_u16(caffeine_blood_mg),
    .gut_mg = float_to_u16(caffeine_gut_mg),
    .pending_mg = float_to_u16(caffeine_pending_mg)
  };
  
  return totals;
}

void add_drink(int16_t miligrams, int8_t ingestion_duration) {
  ingestion i = {
    .dose = miligrams,
    .duration = ingestion_duration,
    .time = time(NULL)
  };
  prv_ingestion_push(i);
}

void calculate_caffeine_stats() {
  caffeine_stats_t stats;
  
  // calculate
  stats.peak_mg = 0;
  stats.peak_time = 0;
  stats.sleep_time = 0;
  
  persist_write_data(STORAGE_KEY_CAFFEINE_STATS, &stats, sizeof(stats));
}

caffeine_stats_t get_caffeine_stats() {
  if (!persist_exists(STORAGE_KEY_CAFFEINE_STATS))
    calculate_caffeine_stats();
  
  caffeine_stats_t stats_buffer;
  persist_read_data(STORAGE_KEY_CAFFEINE_STATS, &stats_buffer, sizeof(stats_buffer));
  
  return stats_buffer;
}
