//
// Created by tika on 5/2/24.
//

#include "process.h"

/**
 * prepare data from string
 * @param data data string
 * @param col_count column count
 * @param d delimiter
 * @return data of table format
 */
static Result *prepare_data(const String &data, int col_count, char d) {
    val res = new Result();
    var lines = split_string(data, '\n');
    for (var &line: *lines) {
        val row = new Row();
        var cols = d ? split_string(line, d) : split_string_by_spaces(line);

        val size = cols->size();
        if (size > col_count) {
            for (var i = col_count; i < size; i++) {
                cols->at(col_count - 1) += "|" + cols->at(i);
            }
        }
        cols->resize(col_count);

        for (val &col: *cols) {
            row->emplace_back(new Cell(trim(col)));
        }

        res->emplace_back(row);
    }

    return res;
}

static void free_row(Row *row) {
    if (!row) {
        return;
    }
    for (val &cell: *row) {
        delete cell;
    }
    delete row;
}

static void free_result(Result *result) {
    if (!result) {
        return;
    }
    for (val &row: *result) {
        free_row(row);
    }
    delete result;
}

/**
 * repair to number type of cell
 * @param cell the cell
 */
static void repair_cell(Cell *cell) {
    if (!cell) {
        return;
    }

    if (cell->type == D_BOOL) {
        show_error("require numbers, but got bool");
    }

    if (cell->type == D_STRING) {
        if (is_double(cell->text)) {
            cell->type = D_NUMBER;
            cell->number = stod(cell->text);
        } else {
            show_error("require numbers, but got string");
        }
    }
}

/**
 * repair type of two cells
 * @param left left cell
 * @param right right cell
 * @return matched type
 */
static DataType repair_type(Cell *left, Cell *right) {
    repair_cell(left);
    repair_cell(right);

    return D_NUMBER;
}

/**
 * evaluate mathematical expression
 * @param cell result cell
 * @param left left cell
 * @param right right cell
 * @param atype expression type
 */
static void calc_math(Cell *cell, Cell *left, Cell *right, ASTType atype) {
    cell->type = repair_type(left, right);

    switch (atype) {
        case A_ADD:
            cell->number = left->number + right->number;
            break;
        case A_SUB:
            cell->number = left->number - right->number;
            break;
        case A_MUL:
            cell->number = left->number * right->number;
            break;
        case A_DIV:
            cell->number = left->number / right->number;
            break;
        case A_MOD:
            cell->number = fmod(left->number, right->number);
            break;
        default:
            break; // make compiler happy
    }
}

/**
 * evaluate logical expression
 * @param cell result cell
 * @param left left cell
 * @param right right cell
 * @param atype expression type
 */
static void calc_logic(Cell *cell, Cell *left, Cell *right, ASTType atype) {
    cell->type = D_BOOL;
    switch (atype) {
        case A_EQ:
        case A_NE:
            if (left->type == D_STRING && right->type == D_STRING) {
                if (atype == A_EQ) {
                    cell->number = left->text == right->text;
                } else {
                    cell->number = left->text != right->text;
                }
                break;
            } else if (left->type == D_STRING || right->type == D_STRING) {
                repair_type(left, right);
            }

            if (atype == A_EQ) {
                cell->number = left->number == right->number;
            } else {
                cell->number = left->number != right->number;
            }
            break;
        case A_LT:
            cell->number = left->number < right->number;
            break;
        case A_GT:
            cell->number = left->number > right->number;
            break;
        case A_LE:
            cell->number = left->number <= right->number;
            break;
        case A_GE:
            cell->number = left->number >= right->number;
            break;
        case A_AND:
            cell->number = (bool) left->number && (bool) right->number;
            break;
        case A_OR:
            cell->number = (bool) left->number || (bool) right->number;
            break;
        case A_NOT:
            cell->number = left->number == 0;
            break;
        case A_ISNULL:
            if (left->type == D_STRING) {
                cell->number = left->text.empty();
            } else {
                cell->number = left->text == "<null>";
            }
            break;
        default:
            break; // make compiler happy
    }
}

/**
 * evaluate like expression
 * @param cell result cell
 * @param data data cell
 * @param like like cell
 */
static void calc_like(Cell *cell, Cell *data, Cell *like) {
    var reg_exp = String("^");
    var esc = false;
    for (char c: like->text) {
        if (esc) {
            reg_exp += String(1, c);
            esc = false;
        } else if (c == '\\') {
            esc = true;
        } else if (c == '%') {
            reg_exp += ".*";
        } else if (c == '_') {
            reg_exp += ".";
        } else {
            reg_exp += c;
        }
    }
    reg_exp += "$";

    val reg = Regex(reg_exp);
    cell->type = D_BOOL;
    cell->number = std::regex_match(data->text, reg);
}

/**
 * evaluate expression
 * @param row data to be evaluated
 * @param exp expression
 * @return result cell
 * @todd some nodes are not implemented
 */
static Cell *evaluate(Row *row, ASTNode *exp) {
    if (!exp) {
        return null;
    }
    val left = evaluate(row, exp->left);
    val right = evaluate(row, exp->right);

    val cell = new Cell();
    switch (exp->atype) { // never be A_JOIN
        /*
         * A_FUNC_CALL, A_PARAM, A_LITERAL, A_COLUMN,
         * A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
         * A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_AND, A_OR,
         * A_NEGATE, A_NOT, A_ISNULL, A_LIKE,
         */
        case A_FUNC_CALL:
        case A_PARAM:
            break; // to be implemented
        case A_LITERAL:
            cell->type = exp->dtype;
            cell->number = exp->number;
            cell->text = exp->text;
            break;
        case A_COLUMN:
            cell->type = row->at((int) exp->number)->type;
            cell->number = row->at((int) exp->number)->number;
            cell->text = row->at((int) exp->number)->text;
            break;
        case A_ADD:
        case A_SUB:
        case A_MUL:
        case A_DIV:
        case A_MOD:
        case A_NEGATE:
            calc_math(cell, left, right, exp->atype);
            break;
        case A_EQ:
        case A_NE:
        case A_LT:
        case A_GT:
        case A_LE:
        case A_GE:
        case A_AND:
        case A_OR:
        case A_NOT:
        case A_ISNULL:
            calc_logic(cell, left, right, exp->atype);
            break;
        case A_LIKE:
            calc_like(cell, left, right);
            break;
        default:
            break; // make compiler happy
    }

    return cell;
}

/**
 * check if the data satisfies the condition
 * @param row the data
 * @param where the condition AST
 * @return true if the data satisfies the condition, false otherwise
 */
static bool is_satisfy(Row *row, ASTNode *where) {
    val cell = evaluate(row, where);
    val value = cell->number > 0;
    delete cell;
    return value;
}

/**
 * apply FROM clause
 * @param from from clause
 * @param data data set of table
 * @return data of table format
 */
static Result *apply_from(ASTNode *from, Map<String, Result *> *data) {
    if (!from->left) {
        return data->at(from->text);
    }

    val res = new Result();
    val left = apply_from(from->left, data);
    val right = data->at(from->text);

    for (val &row1: *left) {
        for (val &row2: *right) {
            val new_row = new Row();
            new_row->insert(new_row->end(), row1->begin(), row1->end());
            new_row->insert(new_row->end(), row2->begin(), row2->end());
            if (is_satisfy(new_row, from->right)) {
                res->emplace_back(new_row);
            } else {
                delete new_row;
            }
        }
    }

    return res;
}

/**
 * apply WHERE clause
 * @param where where clause
 * @param from data of table format
 * @return data of table format
 */
static Result *apply_where(ASTNode *where, Result *from) {
    if (!where) {
        return from;
    }

    val res = new Result();
    for (val &row: *from) {
        if (is_satisfy(row, where)) {
            res->emplace_back(row);
        } else {
            free_row(row);
        }
    }

    return res;
}

/**
 * apply GROUP BY clause
 * @param group group clause
 * @param where data of table format
 * @return the grouped data of table format
 */
static Map<String, Result *> *apply_group_by(Vector<ASTNode *> *group, Result *where) {
    val groups = new Map<String, Result *>();
    if (!group) {
        groups->insert({"group--1", where}); // -1 means no group
        return groups;
    }

    // <key, group_name>
    val keys = new Map<String, String>();
    for (var &row: *where) {
        var key = String();
        for (auto &col: *group) {
            key += "├";
            val cell = row->at((int) col->number);
            switch (col->dtype) {
                case D_STRING:
                    key += cell->text;
                    break;
                case D_NUMBER:
                case D_BOOL:
                    key += to_string(cell->number);
                    break;
                default:
                    show_error("Cannot recognize data type of column: " + cell->text);
            }
        }

        val value = "group-" + to_string(groups->size());
        keys->insert({key, value});
        if (groups->count(value)) {
            groups->at(value)->emplace_back(row);
        } else {
            val res = new Result();
            res->emplace_back(row);
            groups->insert({value, res});
        }
    }

    return groups;
}

/**
 * apply SELECT clause
 * @param select select clause
 * @param group_by data of table format in groups
 * @return data of table format
 */
static Result *apply_select(Vector<SelectNode> *select, Map<String, Result *> *group_by) {
    val res = new Result();

    Result *group;
    if (group_by->size() == 1 && group_by->begin()->first == "group--1") {
        group = group_by->at("group--1");

        for (val &row: *group) {
            val new_row = new Row();
            for (val &node: *select) {
                val new_cell = evaluate(row, node.col);
                new_row->emplace_back(new_cell);
            }
            res->emplace_back(new_row);
        }
    }

    return res;
}

/**
 * apply ORDER BY clause
 * @param order order clause
 * @param select data of table format
 * @return the ordered data of table format
 */
static Result *apply_order_by(Vector<OrderNode> *order, Result *select) {
    if (!order) {
        return select;
    }

    sort(select->begin(), select->end(), [order](Row *a, Row *b) -> bool {
        var res = false;

        for (val &node: *order) {
            int od;
            val idx = node.index;
            val type = a->at(idx)->type;

            switch (type) {
                case D_STRING:
                    od = a->at(idx)->text.compare(b->at(idx)->text);
                    break;
                case D_NUMBER:
                case D_BOOL:
                    od = compare_number(a->at(idx)->number, b->at(idx)->number);
                    break;
                default:
                    show_error("Internal error when sorting");
            }

            if (od == 0) {
                continue;
            }

            res = od > 0;
            return res ^ node.asc;
        }

        return false;
    });

    return select;
}

/**
 * apply LIMIT clause
 * @param limit limit clause
 * @param order_by data of table format
 * @return data of table format
 */
static Result *apply_limit(LimitNode *limit, Result *order_by) {
    if (!limit) {
        return order_by;
    }

    if (limit->offset < order_by->size()) {
        order_by->assign(order_by->begin() + limit->offset, order_by->end());

        if (limit->count < order_by->size()) {
            order_by->resize(limit->count);
        }
    } else {
        order_by->clear();
    }

    return order_by;
}

/**
 * apply SELECT statement, ignore the with clause(passed by data)
 * @param stmt select statement
 * @param data data set of table
 * @return data of table format
 */
static Result *apply_stmt(SelectStatement *stmt, Map<String, Result *> *data) {
    val from = apply_from(stmt->from, data);
    val where = apply_where(stmt->where, from);
    val group_by = apply_group_by(stmt->group, where);
    val select = apply_select(stmt->select, group_by);
    val order_by = apply_order_by(stmt->order, select);
    val limit = apply_limit(stmt->limit, order_by);
    return limit;
}

/**
 * apply WITH clause
 * @param with with clause
 * @param pre_data prepared table data
 * @return data set of table
 */
static Map<String, Result *> *apply_with(Vector<WithNode> *with, Result *pre_data) {
    val res = new Map<String, Result *>;
    res->insert({"std", pre_data});

    if (!with) {
        return res;
    }

    for (val &node: *with) {
        val data = apply_stmt(node.stmt, res);
        res->insert({node.as, data});
    }

    return res;
}

/**
 * Apply one query to the data
 * @param query the query AST
 * @param data the origin data to be processed, not included the title
 * @param col_count the column count of the data
 * @param d the delimiter of the data, default is "\\s+"
 * @return the processed data
 */
Result *apply(SelectStatement *query, const String &data, int col_count, char d) {
    if (!query) {
        return null;
    }
    if (data.empty() && !query->tableless()) {
        show_error("Data is empty, but query is not tableless");
    }

    val pre_data = prepare_data(data, col_count, d);
    val with_data = apply_with(query->with, pre_data);
    val res = apply_stmt(query, with_data);

    // free with_data
    for (val &result: *with_data) {
        free_result(result.second);
    }
    with_data->clear();
    delete with_data;

    return res;
}
