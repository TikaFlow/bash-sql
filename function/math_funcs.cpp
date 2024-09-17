//
// Created by tika on 24-9-16.
//

#include <cmath>
#include <random>
#include "funcs.h"

static Cell *tk_mod(Row *row) {
    if (row->size() != 2) {
        show_error("mod: wrong number of arguments");
    }

    val left = row->at(0);
    val right = row->at(1);

    if ((left->type != D_INTEGER && left->type != D_REAL)
        || (right->type != D_INTEGER && right->type != D_REAL)) {
        show_error("mod: arguments must be numbers");
    }

    if (left->text == NONE || right->text == NONE) {
        return new Cell(left->type);
    }

    if (right->number == 0) {
        return new Cell(left->type);
    }

    val cell = new Cell();
    cell->type = left->type;
    cell->type = right->type == D_REAL ? D_REAL : cell->type;
    cell->number = fmod(left->number, right->number);

    return cell;
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
}

void init_math_funcs() {
    ADD_NORMAL(mod, D_NUMBER);
    ADD_NORMAL(rand, D_REAL);
}
