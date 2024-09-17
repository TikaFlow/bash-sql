//
// Created by tika on 24-9-15.
//

#include "funcs.h"

Map<String, Function> FUNCTIONS;

void init_funcs() {
    init_type_funcs();
    init_flow_funcs();
    init_math_funcs();
    init_date_funcs();
    init_string_funcs();
    init_hash_funcs();
    init_agg_funcs();
    init_misc_funcs();
}
