//
// Created by tika on 5/2/24.
//

#include "process.h"

String Cell::to_string() const {
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

static Cell *evaluate(Row *row, ASTNode *exp);

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
            if (is_integer(col)) {
                row->emplace_back(new Cell(D_INTEGER, stoi(col)));
            } else if (is_double(col)) {
                row->emplace_back(new Cell(D_REAL, stod(col)));
            } else if (col == "true" || col == "false") {
                row->emplace_back(new Cell(D_BOOL, col == "true"));
            } else {
                row->emplace_back(new Cell(col));
            }
        }

        res->emplace_back(row);
    }

    return res;
}

/**
 * make new row from two rows
 * @param left left row
 * @param right right row
 * @return new row
 */
static Row *make_row(Row *left, Row *right) {
    val new_row = new Row();

    if (left) {
        for (val &cell: *left) {
            val new_cell = new Cell(*cell);
            new_row->emplace_back(new_cell);
        }
    }

    if (right) {
        for (val &cell: *right) {
            val new_cell = new Cell(*cell);
            new_row->emplace_back(new_cell);
        }
        return new_row;
    }

    return new_row;
}

/**
 * free memory of row
 * @param row the row
 */
static void free_row(Row *&row) {
    if (!row) {
        return;
    }

    for (var &cell: *row) {
        if (!cell) {
            continue;
        }
        delete cell;
        cell = null;
    }

    delete row;
    row = null;
}

/**
 * free memory of result
 * @param result the result
 */
static void free_result(Result *result) {
    if (!result) {
        return;
    }

    for (var &row: *result) {
        free_row(row);
    }

    delete result;
    result = null;
}

/**
 * repair to target type of cell
 * @param cell the cell
 * @param target target type
 * @return the type of result
 */
static DataType repair_cell(Cell *cell, DataType target) {
    if (!cell) {
        return D_NONE;
    }

    switch (target) {
        case D_INTEGER:
        case D_REAL:
            if (cell->type == D_BOOL) {
                show_error("incompatible type for arithmetic operation");
            } else if (cell->type == D_STRING) {
                if (is_integer(cell->text)) {
                    cell->type = D_INTEGER;
                    cell->number = stoi(cell->text);
                } else if (is_double(cell->text)) {
                    cell->type = D_REAL;
                    cell->number = stod(cell->text);
                } else {
                    show_error("cannot convert string to number");
                }
            } else {
                return cell->type;
            }
        case D_STRING:
            if (cell->type > D_STRING) {
                cell->text = to_string(cell->number);
            }
            return D_STRING;
        default:
            return D_NONE; // make compiler happy
    }
}

/**
 * repair type of two cells
 * @param left left cell
 * @param right right cell
 * @param op operate type
 * @return the type of result
 */
static DataType repair_type(Cell *left, Cell *right, ASTType op) {
    val type = (op == A_EQ || op == A_NE) ? D_STRING : D_INTEGER;
    val ltype = repair_cell(left, type);
    val rtype = repair_cell(right, type);

    if (op == A_EQ || op == A_NE) {
        return D_BOOL;
    }

    if (op == A_NEGATE) {
        return ltype;
    }

    if (ltype == D_REAL || rtype == D_REAL) {
        return D_REAL;
    }

    return D_INTEGER;
}

/**
 * evaluate function call
 * @note the function node is like:
 *                  func_call(name)
 *                  /       \
 *              param_node  null
 *              /       \
 *  prev_param_node   param_exp
 * @param row data to be evaluated
 * @param func_node function node
 * @return result cell
 */
static Cell *calc_func_call(Row *row, ASTNode *func_node) {
    val params = new Row();
    val stc = new Stack<Cell *>();

    var param_node = func_node->left;
    while (param_node) {
        stc->push(evaluate(row, param_node->right));
        param_node = param_node->left;
    }
    while (!stc->empty()) {
        params->emplace_back(stc->top());
        stc->pop();
    }

    val func = FUNCTIONS.at(func_node->text).pointer;
    return func(params);
}

/**
 * evaluate mathematical expression
 * @param cell result cell
 * @param left left cell
 * @param right right cell
 * @param atype expression type
 */
static void calc_math(Cell *cell, Cell *left, Cell *right, ASTType atype) {
    cell->type = repair_type(left, right, atype);

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
        case A_NEGATE:
            cell->number = -left->number;
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
    val cmp = compare_number(left->number, right ? right->number : 0);
    switch (atype) {
        case A_EQ:
            repair_type(left, right, atype);
            cell->number = left->text == right->text;
            break;
        case A_NE:
            repair_type(left, right, atype);
            cell->number = left->text != right->text;
            break;
        case A_LT:
            cell->number = cmp < 0;
            break;
        case A_GT:
            cell->number = cmp > 0;
            break;
        case A_LE:
            cell->number = cmp <= 0;
            break;
        case A_GE:
            cell->number = cmp >= 0;
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
                cell->number = left->text == NONE;
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

    if (exp->atype == A_FUNC_CALL) { // A_PARAM is in the subtree
        return calc_func_call(row, exp);
    }

    val left = evaluate(row, exp->left);
    val right = evaluate(row, exp->right);

    val cell = new Cell();
    switch (exp->atype) { // never be A_JOIN
        /*
         * A_LITERAL, A_COLUMN,
         * A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
         * A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_AND, A_OR,
         * A_NEGATE, A_NOT, A_ISNULL, A_LIKE,
         */
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
    val res = new Result();
    if (!from) {
        res->emplace_back(null);
        return res;
    }

    val left = apply_from(from->left, data);
    val right = data->at(from->text);

    for (val &row1: *left) {
        for (val &row2: *right) {
            var new_row = make_row(row1, row2);
            if (is_satisfy(new_row, from->right)) {
                res->emplace_back(new_row);
            } else {
                free_row(new_row);
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
    for (var &row: *from) {
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
            if (col->dtype == D_STRING) {
                key += cell->text;
            } else {
                key += to_string(cell->number);
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

        free_result(group);
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

            if (type == D_STRING || a->at(idx)->text == NONE) {
                od = a->at(idx)->text.compare(b->at(idx)->text);
            } else {
                od = compare_number(a->at(idx)->number, b->at(idx)->number);
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
        for_each(order_by->begin(), order_by->begin() + limit->offset, free_row);
        order_by->assign(order_by->begin() + limit->offset, order_by->end());

        if (limit->count < order_by->size()) {
            for_each(order_by->begin() + limit->count, order_by->end(), free_row);
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
