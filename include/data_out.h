//
// Created by tika on 24-5-11.
//

#ifndef BASH_SQL_DATA_OUT_H
#define BASH_SQL_DATA_OUT_H

#include "getopt_util.h"
#include "defs.h"

void print_data(Result *data, Vector<SelectNode> *select, bool print_title, bool print_line_no);

#endif //BASH_SQL_DATA_OUT_H
