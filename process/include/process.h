//
// Created by tika on 5/2/24.
//

#ifndef BASH_SQL_PROCESS_H
#define BASH_SQL_PROCESS_H

#include <cmath>
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

    Cell(const Cell &cell) = default;

    Cell(DataType type, double num) : type(type), number(num) {};

    explicit Cell(const String &str) : type(D_STRING), text(str) {};

    String to_string() const {
        switch (type) {
            case D_BOOL:
                return number == 0 ? "false" : "true";
            case D_INTEGER:
            case D_REAL:
                return cut_tail(::to_string(number));
            case D_STRING:
                return text;
            default:
                return ""; // make compiler happy
        }
    }
};

Result *apply(SelectStatement *query, const String &data, int col_count, char d);

#endif //BASH_SQL_PROCESS_H
