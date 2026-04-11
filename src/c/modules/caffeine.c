#include <stdint.h>
#include "src/c/helpers.h"
#include "src/c/modules/caffeine.h"
#include "src/c/modules/settings.h"

#define DRINKS_BUF_SIZE  32

#define F         24
#define ONE_FP    (1LL << F)
#define HALF_FP   (1LL << (F - 1))
#define LN2_FP    11629080  // (int32_t)(ln(2) * ONE_FP + 0.5)
#define INV6_FP   2796203   // (int32_t)(ONE_FP / 6.0 + 0.5)
#define DELTA_T   (20 * SECONDS_PER_MINUTE)
#define N_LUT     128
#define NG_IN_MG  1000000
#define CUTOFF    100000

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

static drink_t s_drinks_buf[DRINKS_BUF_SIZE];
static drinks_meta_t s_drinks_meta;

static rate_constants_t s_rates;

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

static void prv_build_LUT(int32_t k_fp, int32_t LUT[N_LUT]) {
    // x1 = k_fp * DELTA_T 
    int64_t x1 = ((int64_t)k_fp * DELTA_T);
    
    // base step E1 = e^(-k * dT)
    int32_t E1_fp = prv_exp_taylor_neg((int32_t)x1);

    LUT[0] = ONE_FP;
    if (N_LUT > 1) LUT[1] = E1_fp;
    
    // accumulate by repeated multiplication with rounding
    for (int n = 2; n < N_LUT; n++) {
        LUT[n] = (int32_t)((((int64_t)LUT[n-1] * E1_fp) + HALF_FP) >> F);
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

  // remainder factor e^{-k * rem} via 3-term Taylor series
  int64_t y_fp = ((int64_t)k_fp * rem);
  int64_t y2 = (y_fp * y_fp + HALF_FP) >> F;
  int64_t y3 = (y2 * y_fp + HALF_FP) >> F;
  
  // taylor = 1 - y + y^2/2 - y^3/6
  int64_t taylor = ONE_FP - y_fp + (y2 >> 1) - ((y3 * INV6_FP + HALF_FP) >> F);
  if (taylor < 0) taylor = 0;
  
  val = (val * taylor + HALF_FP) >> F;
  return (int32_t)val;
}

static drink_t* prv_compute_drink_contribution(time_t current_time, drink_t *drink, caffeine_totals_t *totals) {
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
    
    G_contrib /= NG_IN_MG;
    B_contrib /= NG_IN_MG;

    totals->gut_mg += G_contrib;
    totals->blood_mg += B_contrib;
    totals->pending_mg += (int32_t)(((int64_t)drink->dose_mg * frac + HALF_FP) >> F);
    
    return prv_get_drink_next(drink);
  } else {
    // drink finished
            
    // calculate the exact state at the moment the drink ended
    int32_t E_a_d = prv_eval_exp(s_rates.ka_fp, drink->duration_min * SECONDS_PER_MINUTE, LUT_ka);
    int32_t E_e_d = prv_eval_exp(s_rates.ke_fp, drink->duration_min * SECONDS_PER_MINUTE, LUT_ke);
            
    int32_t G_end = (int32_t)((R_div_ka * (ONE_FP - E_a_d) + HALF_FP) >> F);
    int32_t B_end = (int32_t)((R_div_ke * (ONE_FP - E_e_d) + HALF_FP) >> F);
    int32_t diff_term_end = (int32_t)((R_div_diff * (E_e_d - E_a_d) + HALF_FP) >> F);
    B_end -= diff_term_end;

    int32_t tau_after = tau - (drink->duration_min * SECONDS_PER_MINUTE);
    int32_t E_a_rem = prv_eval_exp(s_rates.ka_fp, tau_after, LUT_ka);
    int32_t E_e_rem = prv_eval_exp(s_rates.ke_fp, tau_after, LUT_ke);

    int32_t G_contrib = (int32_t)(((int64_t)G_end * E_a_rem + HALF_FP) >> F);
    int32_t B_contrib = ((int64_t)B_end * E_e_rem + HALF_FP) >> F;

    int64_t gut_absorbed_term = ((int64_t)s_rates.factor_fp * G_end + HALF_FP) >> F;
    int64_t diff_exp = E_a_rem - E_e_rem;
    if (s_rates.ka_fp > s_rates.ke_fp) {
        B_contrib -= (int32_t)((gut_absorbed_term * diff_exp + HALF_FP) >> F);
    } else {
        B_contrib += (int32_t)((gut_absorbed_term * diff_exp + HALF_FP) >> F);
    }
    
    if (G_contrib + B_contrib <= CUTOFF) {
      G_contrib /= NG_IN_MG;
      B_contrib /= NG_IN_MG;
  
      totals->gut_mg += G_contrib;
      totals->blood_mg += B_contrib;
      
      drink_t *next = prv_get_drink_next(drink);
      prv_drink_remove(drink);
      
      return next;
    } else {
      G_contrib /= NG_IN_MG;
      B_contrib /= NG_IN_MG;
  
      totals->gut_mg += G_contrib;
      totals->blood_mg += B_contrib;
      totals->pending_mg += (drink->dose_mg - G_contrib - B_contrib);
      
      return prv_get_drink_next(drink);
    }
  }
}

// ---- CAFFEINE FUNCTIONS END -----


// ----- PUBLIC FUNCTIONS BEGIN -----

void caffeine_init() {
  prv_init_drinks();
  prv_compute_rates(settings.half_life_elim, settings.half_life_abs);
  prv_init_LUTs();
}

void add_drink(int16_t miligrams, int8_t ingestion_duration) {
  drink_t drink = {
    .dose_mg = miligrams,
    .duration_min = ingestion_duration,
    .start_time = time(NULL)
  };
  
  prv_drink_insert(drink);
}

void metabolism_update_settings(int16_t hl_elim_m, int8_t hl_abs_m) {
  prv_compute_rates(hl_elim_m, hl_abs_m);
  prv_build_LUT(s_rates.ka_fp, LUT_ka);
  prv_build_LUT(s_rates.ke_fp, LUT_ke);
}

void calculate_caffeine_stats() {
  
}

caffeine_totals_t get_caffeine_totals(){
  caffeine_totals_t totals = {0, 0, 0};
  time_t current_time = time(NULL);
  drink_t *drink = prv_get_drink_head();
  
  while (drink != NULL) {
    drink = prv_compute_drink_contribution(current_time, drink, &totals);
  }
  
  return totals;
}

caffeine_stats_t get_caffeine_stats() {
  caffeine_stats_t stats = {0,0,0};
  return stats;
}