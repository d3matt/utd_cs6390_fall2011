#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

uint32_t * string_to_int(const char * str, uint32_t &ret);

bool valid_AS(uint32_t AS);
bool valid_router(uint32_t router);
