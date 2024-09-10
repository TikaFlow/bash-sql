//
// Created by tika on 5/2/24.
//

#ifndef BASH_SQL_PROCESS_H
#define BASH_SQL_PROCESS_H

#include <iostream>
#include <string>
#include "getopt_util.h"
#include "defs.h"

typedef struct Cell Cell;
using Line = Vector<Cell *>;

struct Cell {
    DataType type;
    union {
        long l;
        double d;
        bool b;
    };
    String s;

    explicit Cell(long l) : type(D_INT), l(l) {};

    explicit Cell(double d) : type(D_REAL), d(d) {};

    explicit Cell(bool b) : type(D_BOOL), b(b) {};

    explicit Cell(String s) : type(D_STRING), s(std::move(s)), l(0) {};
};

Vector<Line*> *apply(SelectStatement *query, const String &data, int col_count, char d);

#endif //BASH_SQL_PROCESS_H
