#pragma once
#include <stdint.h>
#include "src/c/modules/settings.h"

void push_window_group(int16_t *caff_content, char (*titles)[STR_MAXLEN + 1]);
