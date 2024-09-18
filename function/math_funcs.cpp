//
// Created by tika on 24-9-16.
//

#include <cmath>
#include <random>
#include "funcs.h"

/**
 * check if the argument at index is a number
 * @param row params
 * @param name function name
 * @param index index of the argument, starting from 1
 */
static void check_number(Row *row, const String &name, int index) {
    if (row->size() < index) {
        show_error(name + ": wrong number of arguments");
    }

    val cell = row->at(index - 1);
    if (cell->type != D_INTEGER && cell->type != D_REAL) {
        show_error(name + ": argument at index " + to_string(index) + " must be a number");
    }
}

/**
 * get greatest or least in the params
 * @param row params
 * @param name function name
 * @return result
 */
static Cell *cell_compare(Row *row, const String &name) {
    if (row->empty()) {
        return new Cell(D_INTEGER);
    }

    var res = new Cell();
    res->type = D_INTEGER;
    for (val &cell: *row) {
        if (cell->type == D_BOOL) {
            res->text = NONE;
            break;
        } else if (cell->type == D_STRING) {
            if (cell->text.empty()) {
                res->text = NONE;
                break;
            }
            res->type = D_STRING;
        } else {
            if (cell->text == NONE) {
                res->text = NONE;
                break;
            }
            if (cell->type == D_REAL) {
                res->type = D_REAL;
            }
        }
    }

    if (res->text == NONE) {
        if (res->type == D_STRING) {
            res->text = "";
        }
        return res;
    }

    res->number = row->at(0)->number;
    res->text = row->at(0)->text;
    for (val &cell: *row) {
        var cmp = false;
        if (cell->type == D_STRING) {
            cmp = name == "greatest" ? (cell->text > res->text) : (cell->text < res->text);
            if (cmp) {
                res->text = cell->text;
            }
        } else {
            cmp = name == "greatest" ? (cell->number > res->number) : (cell->number < res->number);
            if (cmp) {
                res->number = cell->number;
            }
        }
    }

    return res;
}

/**
 * execute a math function with 1 argument
 * @param row params
 * @param name who calls this function
 * @param func math function
 * @param type result type
 * @param condition condition for params
 * @return result
 */
static Cell *
math1(Row *row, const String &name, double(*func)(double), DataType type = D_NONE, bool(*condition)(double) = null) {
    check_number(row, name, 1);

    val cell = row->at(0);
    if (cell->text == NONE || (condition && !condition(cell->number))) {
        return new Cell(cell->type);
    }

    if (type != D_NONE) {
        type = D_REAL;
    }
    return new Cell(type, func(cell->number));
}

/**
 * execute a math function with 2 arguments
 * @param row params
 * @param name who calls this function
 * @param func math function
 * @param first first param, if it can be omitted
 * @param second second param, if it can be omitted, first and second can't be non-null at the same time
 * @param condition condition for params
 * @param type result type
 * @return result
 */
static Cell *
math2(Row *row, const String &name, double(*func)(double, double), Cell *first = null, Cell *second = null,
      bool(*condition)(double, double) = null, DataType type = D_NONE) {
    Cell *left, *right;

    check_number(row, name, 1);
    if ((first || second) && row->size() == 1) {
        if (first) {
            left = first;
            right = row->at(0);
        } else {
            left = row->at(0);
            right = second;
        }
    } else if (row->size() == 2) {
        check_number(row, name, 2);
        left = row->at(0);
        right = row->at(1);
    } else {
        show_error(name + ": wrong number of arguments");
    }

    if (type == D_NONE) {
        type = (left->type == D_REAL || right->type == D_REAL) ? D_REAL : D_INTEGER;
    }

    if (left->text == NONE || right->text == NONE || (condition && !condition(left->number, right->number))) {
        return new Cell(type);
    }

    return new Cell(type, func(left->number, right->number));
}

/**
 * execute log function with specified base
 * @param row params
 * @param name who calls this function
 * @param b base
 * @return result
 */
static Cell *log_with_base(Row *row, const String &name, double b = 0) {
    var base = Cell(D_REAL, b == 0 ? M_E : b);
    var new_row = Row({&base, row->at(0)});

    return math2(b == 0 ? row : &new_row, name, [](double b, double x) { return log(x) / log(b); }, &base, null,
                 [](double b, double x) { return b > 1 && x > 0; }, D_REAL);
}

static Cell *tk_abs(Row *row) {
    return math1(row, "abs", fabs);
}

static Cell *tk_acos(Row *row) {
    return math1(row, "acos", acos, D_REAL, [](double x) { return x >= -1 && x <= 1; });
}

static Cell *tk_asin(Row *row) {
    return math1(row, "asin", asin, D_REAL, [](double x) { return x >= -1 && x <= 1; });
}

static Cell *tk_atan(Row *row) {
    return math1(row, "atan", atan);
}

static Cell *tk_ceil(Row *row) {
    return math1(row, "ceil", ceil, D_INTEGER);
}

static Cell *tk_ceiling(Row *row) {
    return math1(row, "ceiling", ceil, D_INTEGER);
}

static Cell *tk_cos(Row *row) {
    return math1(row, "cos", cos);
}

static Cell *tk_cot(Row *row) {
    return math1(row, "cot", [](double x) { return 1 / tan(x); });
}

static Cell *tk_degrees(Row *row) {
    return math1(row, "degrees", [](double x) { return x * 180 / M_PI; });
}

static Cell *tk_exp(Row *row) {
    return math1(row, "exp", exp);
}

static Cell *tk_floor(Row *row) {
    return math1(row, "floor", floor, D_INTEGER);
}

static Cell *tk_greatest(Row *row) {
    return cell_compare(row, "greatest");
}

static Cell *tk_hex(Row *row) {
    if (row->size() != 1) {
        show_error("hex: wrong number of arguments");
    }

    val cell = row->at(0);
    if (cell->type != D_INTEGER && cell->type != D_STRING) {
        show_error("hex: argument must be a integer or string");
    }

    std::ostringstream oss;
    if (cell->type == D_INTEGER) {
        oss << std::uppercase << std::hex << (int) cell->number;
    } else {
        for (char c: cell->text) {
            oss << std::uppercase << std::hex << (int) c;
        }
    }

    return new Cell(oss.str());
}

static Cell *tk_least(Row *row) {
    return cell_compare(row, "least");
}

static Cell *tk_ln(Row *row) {
    return log_with_base(row, "ln");
}

static Cell *tk_log(Row *row) {
    return log_with_base(row, "log");
}

static Cell *tk_log2(Row *row) {
    return log_with_base(row, "log2", 2);
}

static Cell *tk_log10(Row *row) {
    return log_with_base(row, "log10", 10);
}

static Cell *tk_mod(Row *row) {
    return math2(row, "mod", fmod, null, null, [](double n, double m) { return m != 0; });
}

static Cell *tk_pi(Row *row) {
    if (!row->empty()) {
        show_error("pi: wrong number of arguments");
    }

    return new Cell(D_REAL, M_PI);
}

static Cell *tk_pow(Row *row) {
    return math2(row, "pow", pow);
}

static Cell *tk_power(Row *row) {
    return math2(row, "power", pow);
}

static Cell *tk_radians(Row *row) {
    return math1(row, "radians", [](double x) { return x * M_PI / 180; });
}

static Cell *tk_rand(Row *row) {
    static var seeds = Map<double, double>();
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);

    if (row->empty()) {
        return new Cell(D_REAL, dis(gen));
    } else if (row->size() == 1) {
        val seed = row->at(0);
        if (seed->type != D_INTEGER && seed->type != D_REAL) {
            show_error("rand: seed must be a number");
        }

        if (seed->text == NONE) {
            return new Cell(D_REAL);
        }

        if (seeds.count(seed->number)) {
            return new Cell(D_REAL, seeds.at(seed->number));
        } else {
            val rand = dis(gen);
            seeds.insert({seed->number, rand});
            return new Cell(D_REAL, rand);
        }
    } else {
        show_error("rand: wrong number of arguments");
    }

    return null; // make compiler happy
}

static Cell *tk_round(Row *row) {
    var digit = Cell(D_INTEGER, 0);
    return math2(row, "round", [](double x, double y) { return round(x * pow(10, y)) / pow(10, y); }, null, &digit,
                 [](double x, double d) { return (int) d == d; });
}

static Cell *tk_sign(Row *row) {
    return math1(row, "sign", [](double x) -> double { return compare_number(x, 0); }, D_INTEGER);
}

static Cell *tk_sin(Row *row) {
    return math1(row, "sin", sin);
}

static Cell *tk_sqrt(Row *row) {
    return math1(row, "sqrt", sqrt, D_REAL, [](double x) { return x >= 0; });
}

static Cell *tk_tan(Row *row) {
    return math1(row, "tan", tan);
}

static Cell *tk_truncate(Row *row) {
    var digit = Cell(D_INTEGER, 0);
    return math2(row, "truncate", [](double x, double y) { return trunc(x * pow(10, y)) / pow(10, y); }, null, &digit,
                 [](double x, double d) { return (int) d == d; });
}

void init_math_funcs() {
    ADD_NORMAL(abs, D_NUMBER);
    ADD_NORMAL(acos, D_REAL);
    ADD_NORMAL(asin, D_REAL);
    ADD_NORMAL(atan, D_REAL);
    ADD_NORMAL(ceil, D_INTEGER);
    ADD_NORMAL(ceiling, D_INTEGER);
    ADD_NORMAL(cos, D_REAL);
    ADD_NORMAL(cot, D_REAL);
    ADD_NORMAL(degrees, D_REAL);
    ADD_NORMAL(exp, D_REAL);
    ADD_NORMAL(floor, D_INTEGER);
    ADD_NORMAL(greatest, D_STRING);
    ADD_NORMAL(hex, D_STRING);
    ADD_NORMAL(least, D_STRING);
    ADD_NORMAL(ln, D_REAL);
    ADD_NORMAL(log, D_REAL);
    ADD_NORMAL(log2, D_REAL);
    ADD_NORMAL(log10, D_REAL);
    ADD_NORMAL(mod, D_NUMBER);
    ADD_NORMAL(pi, D_REAL);
    ADD_NORMAL(pow, D_NUMBER);
    ADD_NORMAL(power, D_NUMBER);
    ADD_NORMAL(radians, D_REAL);
    ADD_NORMAL(rand, D_REAL);
    ADD_NORMAL(round, D_NUMBER);
    ADD_NORMAL(sign, D_INTEGER);
    ADD_NORMAL(sin, D_REAL);
    ADD_NORMAL(sqrt, D_REAL);
    ADD_NORMAL(tan, D_REAL);
    ADD_NORMAL(truncate, D_NUMBER);
}
