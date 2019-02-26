#ifndef SIMDJSON_STAGE34_UNIFIED_H
#define SIMDJSON_STAGE34_UNIFIED_H

#include "simdjson/common_defs.h"
#include "simdjson/parsedjson.h"

struct ParsedJson;

void init_state_machine();

WARN_UNUSED
int unified_machine(const uint8_t *buf, size_t len, ParsedJson &pj);

WARN_UNUSED
int unified_machine(const char *buf, size_t len, ParsedJson &pj);

#endif
