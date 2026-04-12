#include <stdint.h>
#include "src/c/helpers.h"
#include "src/c/modules/caffeine.h"
#include "src/c/modules/settings.h"

#define DRINKS_BUF_SIZE  32

#define F             24
#define ONE_FP        (1LL << F)
#define HALF_FP       (1LL << (F - 1))
#define LN2_FP        11629080  // (int32_t)(ln(2) * ONE_FP + 0.5)
#define INV2_FP       8388608   // (int32_t)(ONE_FP / 2.0 + 0.5)
#define INV6_FP       2796203   // (int32_t)(ONE_FP / 6.0 + 0.5)
#define INV24_FP      699051    // (int32_t)(ONE_FP / 24.0 + 0.5)
#define DELTA_T       (25 * SECONDS_PER_MINUTE) // 25min * 128 > 53h 
#define N_LUT         128
#define NG_IN_MG      1000000
#define CUTOFF        100000
#define REF_STEP      8

#define PEAK_SEARCH_INTERVAL   (6 * SECONDS_PER_HOUR)
#define PEAK_SEARCH_ITER       20
#define SLEEP_SEARCH_INTERVAL  SECONDS_PER_DAY
#define SLEEP_SEARCH_ITER      16
typedef struct {
  int32_t ka_fp;
  int32_t ke_fp;
  int64_t inv_ka_fp;
  int64_t inv_ke_fp;
  int32_t diff_fp;
  int64_t inv_diff_fp;
  int32_t factor_fp;
} rate_constants_t;

typedef struct {
  time_t start_time;
  uint16_t dose_mg;
  uint8_t duration_min;
} drink_t;

typedef struct {
  uint8_t count;
  uint8_t head;
} drinks_meta_t;

typedef struct {
  time_t start_time;
  uint32_t d_sec;
  int32_t R_ng_per_s;
  int32_t E_a_d_fp;
  int32_t E_e_d_fp;
  int32_t G_end_ng;
  int32_t B_end_ng;
  int64_t dB_coeff_unscaled;
} drink_peak_cache_t;

typedef struct {
  uint32_t pending_ng;
  uint32_t gut_ng;
  uint32_t blood_ng;
} prv_caffeine_totals_t;

static drink_t s_drinks_buf[DRINKS_BUF_SIZE];
static drinks_meta_t s_drinks_meta;

static rate_constants_t s_rates;

static caffeine_stats_t s_stats;

static int32_t LUT_ka[N_LUT];
static int32_t LUT_ke[N_LUT];


// ----- DRINKS BUFFER FUNCTIONS BEGIN -----
static drink_t* prv_get_drink_head() {
  return (s_drinks_meta.count == 0) ? NULL : &s_drinks_buf[s_drinks_meta.head];
}

static void prv_save_drinks() {
  persist_write_data(STORAGE_KEY_DRINKS_BUF, s_drinks_buf, sizeof(s_drinks_buf));
  persist_write_data(STORAGE_KEY_DRINKS_META, &s_drinks_meta, sizeof(s_drinks_meta));
}

static drink_t* prv_get_drink_next(drink_t *current) {
  if (s_drinks_meta.count <= 1 || current == NULL) return NULL;
  
  int current_idx = (int)(current - s_drinks_buf);
  int tail_idx = (s_drinks_meta.head - s_drinks_meta.count + 1 + DRINKS_BUF_SIZE) % DRINKS_BUF_SIZE;
  if (current_idx == tail_idx) return NULL;

  return &s_drinks_buf[(current_idx - 1 + DRINKS_BUF_SIZE) % DRINKS_BUF_SIZE];
}

static void prv_drink_insert(drink_t new) {
  s_drinks_meta.head = (s_drinks_meta.head + 1) % DRINKS_BUF_SIZE;
  s_drinks_buf[s_drinks_meta.head] = new;
    
  if (s_drinks_meta.count < DRINKS_BUF_SIZE) {
    s_drinks_meta.count++;
  }
  
  prv_save_drinks();
}

static bool prv_drink_remove(drink_t *target) {
  if (s_drinks_meta.count == 0 || target == NULL) return false;
  
  int target_idx = (int)(target - s_drinks_buf);
  int current = target_idx;
  int items_to_move;
  
  if (target_idx <= s_drinks_meta.head) {
    items_to_move = s_drinks_meta.head - target_idx;
  } else {
    items_to_move = (DRINKS_BUF_SIZE - target_idx) + s_drinks_meta.head;
  }
  
  for (int i = 0; i < items_to_move; i++) {
    int next_idx = (current + 1) % DRINKS_BUF_SIZE;
    s_drinks_buf[current] = s_drinks_buf[next_idx];
    current = next_idx;
  }
  
  s_drinks_meta.head = (s_drinks_meta.head - 1 + DRINKS_BUF_SIZE) % DRINKS_BUF_SIZE;
  s_drinks_meta.count--;
    
  prv_save_drinks();
  return true;
}

static void prv_init_drinks () {
  if (persist_exists(STORAGE_KEY_DRINKS_BUF) && persist_exists(STORAGE_KEY_DRINKS_META)) {
    persist_read_data(STORAGE_KEY_DRINKS_BUF, &s_drinks_buf, sizeof(s_drinks_buf));
    persist_read_data(STORAGE_KEY_DRINKS_META, &s_drinks_meta, sizeof(s_drinks_meta));
  } else {
    s_drinks_meta.count = 0;
    s_drinks_meta.head = -1;
  }
}

// ----- DRINKS BUFFER FUNCTIONS END -----

// ---- CAFFEINE FUNCTIONS BEGIN -----

static void prv_compute_rates(int16_t e_t_half, int8_t a_t_half) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Elimination half-life (min): %d", e_t_half);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Absorbtion half-life (min): %d", a_t_half);
  
  if (e_t_half < 1) e_t_half = 1;
  if (a_t_half < 1) a_t_half = 1;
  
  int32_t e_half_sec = e_t_half * SECONDS_PER_MINUTE;
  int16_t a_half_sec = a_t_half * SECONDS_PER_MINUTE;

  s_rates.ke_fp = (LN2_FP + (e_half_sec / 2)) / e_half_sec;
  s_rates.ka_fp = (LN2_FP + (a_half_sec / 2)) / a_half_sec;

  s_rates.inv_ka_fp = ((1LL << (F + F)) + (s_rates.ka_fp >> 1)) / s_rates.ka_fp;
  s_rates.inv_ke_fp = ((1LL << (F + F)) + (s_rates.ke_fp >> 1)) / s_rates.ke_fp;

  if (s_rates.ka_fp > s_rates.ke_fp) {
    s_rates.diff_fp = s_rates.ka_fp - s_rates.ke_fp;
  } else {
    s_rates.diff_fp = s_rates.ke_fp - s_rates.ka_fp;
  }

  s_rates.inv_diff_fp = ((1LL << (F + F)) + (s_rates.diff_fp >> 1)) / s_rates.diff_fp;
  s_rates.factor_fp = (int32_t)(((int64_t)s_rates.ka_fp * s_rates.inv_diff_fp + HALF_FP) >> F);
}

// 8-term Taylor series for e^(-x)
static int32_t prv_exp_taylor_neg(int32_t x_fp) {
    int64_t result = ONE_FP;
    int64_t term = ONE_FP;
    
    static const int32_t inv_i[9] = {
      0, ONE_FP, ONE_FP/2, ONE_FP/3, ONE_FP/4, ONE_FP/5, ONE_FP/6, ONE_FP/7, ONE_FP/8
    };
  
    for (int i = 1; i <= 8; i++) {
        // term = - (term * x_fp) / (i * 2^F)
        int64_t product = term * x_fp;
        term = -((product + (product < 0 ? -HALF_FP : HALF_FP)) >> F);
        term = (term * inv_i[i] + HALF_FP) >> F;
        result += term;
    }
    
    if (result < 0) result = 0; // clamp to prevent underflow
    return (int32_t)result;
}

static int32_t prv_exp_taylor_for_step(int32_t k_fp, int32_t duration_s) {
    int64_t x_fp = ((int64_t)k_fp * duration_s);
    int64_t x2 = (x_fp * x_fp + HALF_FP) >> F;
    int64_t x3 = (x2 * x_fp + HALF_FP) >> F;
    int64_t x4 = (x3 * x_fp + HALF_FP) >> F;
    int64_t x5 = (x4 * x_fp + HALF_FP) >> F;
    
    // 1 - x + x^2/2 - x^3/6 + x^4/24 - x^5/120
    return ONE_FP - x_fp + ((x2 * INV2_FP + HALF_FP) >> F) 
                         - ((x3 * INV6_FP + HALF_FP) >> F) 
                         + ((x4 * INV24_FP + HALF_FP) >> F)
                         - ((x5 * (ONE_FP/120) + HALF_FP) >> F);
}

static void prv_build_LUT(int32_t k_fp, int32_t LUT[N_LUT]) {
    LUT[0] = ONE_FP;
    int32_t E_step = prv_exp_taylor_for_step(k_fp, DELTA_T);

    for (int n = 1; n < N_LUT; n++) {
        LUT[n] = (int32_t)(((int64_t)LUT[n-1] * E_step + HALF_FP) >> F);
    }
}

static void prv_init_LUTs() {
  prv_build_LUT(s_rates.ka_fp, LUT_ka);
  prv_build_LUT(s_rates.ke_fp, LUT_ke);
}

static int32_t prv_eval_exp(int32_t k_fp, int32_t t, const int32_t LUT[N_LUT]) {
  int64_t accumulated = ONE_FP;
  
  const int32_t block_size = (N_LUT - 1) * DELTA_T;
  const int32_t last_entry = LUT[N_LUT - 1];
  
  while (t >= block_size) {
    accumulated = (accumulated * last_entry + HALF_FP) >> F;
    t -= block_size;
    if (accumulated == 0) return 0;   // became negligible
  }

  int32_t index = t / DELTA_T;
  int32_t rem = t % DELTA_T;
  int32_t base = LUT[index];
  
  int64_t val = (accumulated * base + HALF_FP) >> F;

  if (rem == 0) return (int32_t)val;

  // remainder factor e^{-k * rem} via 4-term Taylor series
  int64_t y_fp = ((int64_t)k_fp * rem);
  int64_t y2 = (y_fp * y_fp + HALF_FP) >> F;
  int64_t y3 = (y2 * y_fp + HALF_FP) >> F;
  int64_t y4 = (y3 * y_fp + HALF_FP) >> F;
  
  // taylor = 1 - y + y^2/2 - y^3/6 + y^4/24
  int64_t taylor = ONE_FP - y_fp + ((y2 * INV2_FP + HALF_FP) >> F)
                   - ((y3 * INV6_FP + HALF_FP) >> F)
                   + ((y4 * INV24_FP + HALF_FP) >> F);
  
  if (taylor < 0) taylor = 0;
  
  val = (val * taylor + HALF_FP) >> F;
  return (int32_t)val;
}

static bool prv_compute_drink_contribution(time_t current_time, drink_t *drink, prv_caffeine_totals_t *totals) {
  if (drink->start_time > current_time) return true;
  
  uint32_t tau = current_time - drink->start_time;
  uint32_t dose = drink->dose_mg * NG_IN_MG;
  
  int32_t R = dose / ((int32_t)drink->duration_min * SECONDS_PER_MINUTE);
  int64_t R_div_ka = ((int64_t)R * s_rates.inv_ka_fp + HALF_FP) >> F;
  int64_t R_div_ke = ((int64_t)R * s_rates.inv_ke_fp + HALF_FP) >> F;
  int64_t R_div_diff = ((int64_t)R * s_rates.inv_diff_fp + HALF_FP) >> F;
  
  if (tau <= (drink->duration_min * SECONDS_PER_MINUTE)) {
    // drink in progress
    int32_t E_a_tau = prv_eval_exp(s_rates.ka_fp, tau, LUT_ka);
    int32_t E_e_tau = prv_eval_exp(s_rates.ke_fp, tau, LUT_ke);

    int32_t G_contrib = (int32_t)((R_div_ka * (int64_t)(ONE_FP - E_a_tau) + HALF_FP) >> F);
    int32_t B_contrib = (int32_t)((R_div_ke * (int64_t)(ONE_FP - E_e_tau) + HALF_FP) >> F);
            
    int32_t diff_contrib = (int32_t)((R_div_diff * (int64_t)(E_a_tau - E_e_tau) + HALF_FP) >> F);
    
    // for pending
    int64_t frac = ((int64_t)((int32_t)drink->duration_min * SECONDS_PER_MINUTE - tau) << F)
                   / ((int32_t)drink->duration_min * SECONDS_PER_MINUTE);
    
    if (s_rates.ka_fp > s_rates.ke_fp) {
      B_contrib += diff_contrib;
    } else {
      B_contrib -= diff_contrib;
    }
    
    totals->gut_ng += G_contrib;
    totals->blood_ng += B_contrib;
    totals->pending_ng += (uint32_t)((dose * frac + HALF_FP) >> F);
    
    return true;
  } else {
    // drink finished
            
    int32_t E_a_d = prv_eval_exp(s_rates.ka_fp, drink->duration_min * SECONDS_PER_MINUTE, LUT_ka);
    int32_t E_e_d = prv_eval_exp(s_rates.ke_fp, drink->duration_min * SECONDS_PER_MINUTE, LUT_ke);
            
    int32_t G_end = (int32_t)((R_div_ka * (ONE_FP - E_a_d) + HALF_FP) >> F);
    int32_t B_end = (int32_t)((R_div_ke * (ONE_FP - E_e_d) + HALF_FP) >> F);
    int32_t diff_term_end = (int32_t)((R_div_diff * (E_e_d - E_a_d) + HALF_FP) >> F);
    B_end -= diff_term_end;

    int32_t tau_after = tau - (drink->duration_min * SECONDS_PER_MINUTE);
    int32_t E_a_rem = prv_eval_exp(s_rates.ka_fp, tau_after, LUT_ka);
    int32_t E_e_rem = prv_eval_exp(s_rates.ke_fp, tau_after, LUT_ke);

    int32_t G_contrib = (int32_t)(((int64_t)G_end * (int64_t)E_a_rem + HALF_FP) >> F);
    int32_t B_contrib = ((int64_t)B_end * (int64_t)E_e_rem + HALF_FP) >> F;

    int64_t gut_absorbed_term = ((int64_t)s_rates.factor_fp * G_end + HALF_FP) >> F;
    int64_t diff_exp = E_a_rem - E_e_rem;
    if (s_rates.ka_fp > s_rates.ke_fp) {
      B_contrib -= (int32_t)((gut_absorbed_term * diff_exp + HALF_FP) >> F);
    } else {
      B_contrib += (int32_t)((gut_absorbed_term * diff_exp + HALF_FP) >> F);
    }
    
    // im desperate, random clamp
    if (G_contrib < 0) G_contrib = 0;
    if (B_contrib < 0) B_contrib = 0;
    
    if (G_contrib + B_contrib <= CUTOFF) {
      totals->gut_ng += G_contrib;
      totals->blood_ng += B_contrib;
        
      return false;
    } else {
      totals->gut_ng += G_contrib;
      totals->blood_ng += B_contrib;
      
      return true;
    }
  }
}

static prv_caffeine_totals_t prv_get_caffeine_totals(time_t current_time, bool simulation) {
  prv_caffeine_totals_t totals = {
    .blood_ng = 0,
    .gut_ng = 0, 
    .pending_ng = 0
  };
  drink_t *drink = prv_get_drink_head();
    
  while (drink != NULL) {
    if (prv_compute_drink_contribution(current_time, drink, &totals) || simulation) {
      drink = prv_get_drink_next(drink);
    } else {
      bool last = prv_get_drink_next(drink) == NULL;
      prv_drink_remove(drink);
      if (last) break;
    }
  }
  
  return totals;
}

// ----- CAFFEINE STATS BEGIN -----

static int prv_get_global_derivative_sign(time_t t, const drink_peak_cache_t *cache, int count) {
  int64_t total_slope = 0;

  for (int i = 0; i < count; i++) {
    const drink_peak_cache_t *d = &cache[i];
    if (t < d->start_time) continue;

    uint32_t tau = t - d->start_time;
    if (tau <= d->d_sec) {
      int32_t E_a = prv_eval_exp(s_rates.ka_fp, tau, LUT_ka);
      int32_t E_e = prv_eval_exp(s_rates.ke_fp, tau, LUT_ke);
      int32_t diff = (s_rates.ka_fp > s_rates.ke_fp) ? (E_e - E_a) : (E_a - E_e);
      total_slope += (d->dB_coeff_unscaled * diff) >> F;
    } else {
      uint32_t tau_after = tau - d->d_sec;
      int32_t E_a_rem = prv_eval_exp(s_rates.ka_fp, tau_after, LUT_ka);
      int32_t E_e_rem = prv_eval_exp(s_rates.ke_fp, tau_after, LUT_ke);

      // term1 = -ke * B_end * e^-ke*tau
      int64_t term1 = -((int64_t)d->B_end_ng * s_rates.ke_fp) >> F;
      term1 = (term1 * E_e_rem) >> F;

      // term2 = (ka * G_end / (ka-ke)) * (ke*e^-ke*tau - ka*e^-ka*tau)
      int64_t g_coeff = ((int64_t)s_rates.factor_fp * d->G_end_ng) >> F;
      int64_t inner = ((int64_t)s_rates.ke_fp * E_e_rem) >> F;
      inner -= ((int64_t)s_rates.ka_fp * E_a_rem) >> F;
            
      int64_t term2 = (g_coeff * inner) >> F;
      total_slope += (s_rates.ka_fp > s_rates.ke_fp) ? (term1 - term2) : (term1 + term2);
    }
  }

  if (total_slope > 0) return 1;
  if (total_slope < 0) return -1;
  return 0;
}

static void prv_save_caffeine_stats() {
  persist_write_data(STORAGE_KEY_CAFFEINE_STATS, &s_stats, sizeof(s_stats));
}

static void prv_find_caffeine_peak() {
  s_stats.peak_time = 0;
  s_stats.peak_mg = 0;

  int count = s_drinks_meta.count;
  if (count == 0) return;
  
  static drink_peak_cache_t cache[DRINKS_BUF_SIZE];
  int cache_count = 0;
  time_t t_last_end = 0;

  drink_t *drink = prv_get_drink_head();
  
  while (drink && cache_count < DRINKS_BUF_SIZE) {
    drink_peak_cache_t *c = &cache[cache_count++];
    c->start_time = drink->start_time;
    c->d_sec = (uint32_t)drink->duration_min * SECONDS_PER_MINUTE;
    uint32_t dose_ng = drink->dose_mg * NG_IN_MG;
    c->R_ng_per_s = dose_ng / c->d_sec;

    c->E_a_d_fp = prv_eval_exp(s_rates.ka_fp, c->d_sec, LUT_ka);
    c->E_e_d_fp = prv_eval_exp(s_rates.ke_fp, c->d_sec, LUT_ke);
        
    int64_t R = c->R_ng_per_s;
        
    int64_t temp_G = R * (ONE_FP - c->E_a_d_fp);
    c->G_end_ng = (int32_t)((temp_G + (s_rates.ka_fp >> 1)) / s_rates.ka_fp);
        
    int64_t temp_B1 = R * (ONE_FP - c->E_e_d_fp);
    int32_t B_end_1 = (int32_t)((temp_B1 + (s_rates.ke_fp >> 1)) / s_rates.ke_fp);
        
    int64_t temp_B2 = R * (c->E_e_d_fp - c->E_a_d_fp);
    int32_t B_end_2 = (int32_t)((temp_B2 + (s_rates.diff_fp >> 1)) / s_rates.diff_fp);
    c->B_end_ng = B_end_1 - B_end_2;
        
    c->dB_coeff_unscaled = (int64_t)((R * s_rates.ka_fp + (s_rates.diff_fp >> 1)) / s_rates.diff_fp);

    time_t end = drink->start_time + c->d_sec;
    if (end > t_last_end) t_last_end = end;

    drink = prv_get_drink_next(drink);
  }

  time_t search_start = cache[0].start_time;
  time_t search_end = t_last_end + PEAK_SEARCH_INTERVAL;
      
  time_t best_lo = search_start;
  uint32_t max_val = 0;
  const int num_steps = 10;
  time_t step_size = (search_end - search_start) / num_steps;
      
  for (int i = 0; i <= num_steps; i++) {
    time_t check_t = search_start + (i * step_size);
    prv_caffeine_totals_t check_total = prv_get_caffeine_totals(check_t, true);
    if (check_total.blood_ng > max_val) {
      max_val = check_total.blood_ng;
      best_lo = (check_t > step_size) ? check_t - step_size : search_start;
    }
  }
      
  time_t lo = best_lo;
  time_t hi = (best_lo + step_size * 2 > search_end) ? search_end : best_lo + step_size * 2;
      
  for (int i = 0; i < PEAK_SEARCH_ITER; i++) {
    time_t mid = lo + (hi - lo) / 2;
    // Check derivative sign with higher internal precision
    if (prv_get_global_derivative_sign(mid, cache, cache_count) > 0) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  
  prv_caffeine_totals_t final_totals = prv_get_caffeine_totals(lo, true);
  s_stats.peak_time = lo;
  s_stats.peak_mg = final_totals.blood_ng / NG_IN_MG;
}

static void prv_find_sleep_time() {
  if (s_stats.peak_mg < settings.sleep_mg) {
    s_stats.sleep_time = 0;
    return;
  }
  
  uint32_t target = settings.sleep_mg * NG_IN_MG;
  time_t lo = s_stats.peak_time;
  time_t hi = s_stats.peak_time + SLEEP_SEARCH_INTERVAL;
  
  // find 24h interval to search
  while (prv_get_caffeine_totals(hi, true).blood_ng > target) {
    lo = hi;
    hi += SLEEP_SEARCH_INTERVAL;
  }
  
  for (int i = 0; i < SLEEP_SEARCH_ITER; i++) {
    time_t mid = lo + (hi - lo) / 2;
    if (prv_get_caffeine_totals(mid, true).blood_ng >= target) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  
  s_stats.sleep_time = hi;
}

static void prv_calculate_caffeine_stats(bool only_sleep) {
  // find peak
  if (!only_sleep)
    prv_find_caffeine_peak();
  
  // find sleep from peak
  prv_find_sleep_time();
  
  prv_save_caffeine_stats();
}

static void prv_init_caffeine_stats() {
  if (persist_exists(STORAGE_KEY_CAFFEINE_STATS)) {
    persist_read_data(STORAGE_KEY_CAFFEINE_STATS, &s_stats, sizeof(s_stats));
  } else {
    s_stats.peak_time = 0;
    s_stats.sleep_time = 0;
    prv_calculate_caffeine_stats(false);
  }
}


// ---- CAFFEINE FUNCTIONS END -----


// ----- PUBLIC FUNCTIONS BEGIN -----

void caffeine_init() {
  prv_init_drinks();
  prv_compute_rates(settings.half_life_elim, settings.half_life_abs);
  prv_init_LUTs();
  prv_init_caffeine_stats();
}

void add_drink(int16_t miligrams, int8_t ingestion_duration) {
  drink_t drink = {
    .dose_mg = miligrams,
    .duration_min = ingestion_duration,
    .start_time = time(NULL)
  };
  
  prv_drink_insert(drink);
  prv_calculate_caffeine_stats(false);
}

void metabolism_update_settings(int16_t hl_elim_m, int8_t hl_abs_m) {
  prv_compute_rates(hl_elim_m, hl_abs_m);
  prv_build_LUT(s_rates.ka_fp, LUT_ka);
  prv_build_LUT(s_rates.ke_fp, LUT_ke);
  prv_calculate_caffeine_stats(false);
}

void update_sleep() {
  prv_calculate_caffeine_stats(true);
}

caffeine_totals_t get_caffeine_totals() {
  prv_caffeine_totals_t prv_totals = prv_get_caffeine_totals(time(NULL), false);
  caffeine_totals_t totals = {
    .blood_mg = prv_totals.blood_ng / NG_IN_MG,
    .gut_mg = prv_totals.gut_ng / NG_IN_MG,
    .pending_mg = prv_totals.pending_ng / NG_IN_MG,
  };
  
  return totals;
}

caffeine_stats_t get_caffeine_stats() {
  return s_stats;
}