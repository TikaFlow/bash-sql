//
// Created by tika on 5/2/24.
//

#ifndef BASH_SQL_PROCESS_H
#define BASH_SQL_PROCESS_H

#include "getopt_util.h"
#include "defs.h"

typedef struct Cell Cell;
using Row = Vector<Cell *>;
using Result = Vector<Row *>;

struct Cell {
    DataType type;
    double number = 0;
    String text;

    Cell() : type(D_NONE) {};

    Cell(const Cell &cell)  = default;

    explicit Cell(double num) : type(D_NUMBER), number(num) {};

    explicit Cell(const String& str) : type(D_STRING), text(str) {};
};

Result *apply(SelectStatement *query, const String &data, int col_count, char d);

#endif //BASH_SQL_PROCESS_H
