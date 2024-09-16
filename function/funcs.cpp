//
// Created by tika on 24-9-15.
//

#include "funcs.h"

Map<String, Function> FUNCTIONS;

// make compiler happy...
void init_date_funcs() {}

void init_logical_funcs() {}

void init_cast_funcs() {}

void init_misc_funcs() {}

void init_funcs() {
    init_math_funcs();
    init_string_funcs();
    init_date_funcs();
    init_logical_funcs();
    init_cast_funcs();
    init_agg_funcs();
    init_misc_funcs();
}
