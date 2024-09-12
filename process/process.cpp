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

static Cell *evaluate(Row *row, ASTNode *exp) {
    return new Cell(true);
}

/**
 * check if the data satisfies the condition
 * @param row the data
 * @param where the condition AST
 * @return true if the data satisfies the condition, false otherwise
 */
static bool is_satisfy(Row *row, ASTNode *where) {
    val cell = evaluate(row, where);
    val value = cell->b;
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
        return data->at(from->s);
    }

    val res = new Result();
    val left = apply_from(from->left, data);

    for (val &row1: *left) {
        val right = data->at(from->s);
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
static Result *apply_group_by(Vector<ASTNode *> *group, Result *where) {
    if (!group) {
        return where;
    }

    val groups = new Map<String, String>();
    for (var &row: *where) {
        var key = String();
        for (auto &col: *group) {
            key += "├";
            val cell = row->at(col->l);
            switch (col->dtype) {
                case D_STRING:
                    key += cell->s;
                    break;
                case D_INT:
                    key += to_string(cell->l);
                    break;
                case D_REAL:
                    key += to_string(cell->d);
                    break;
                case D_BOOL:
                    key += to_string(cell->b);
                    break;
                default:
                    show_error("Cannot recognize data type of column: " + cell->s);
            }
        }

        groups->insert({key, "group-" + to_string(groups->size())});
        row->emplace_back(new Cell(groups->at(key)));
    }

    return where;
}

static Result *apply_select(Vector<SelectNode> *select, Result *group_by, bool grouped) {
    // in group_by, last cell at every Row is the group key
    return group_by;
}

static Result *apply_order_by(Vector<OrderNode> *order, Result *select) {
    if (!order) {
        return select;
    }
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
    val select = apply_select(stmt->select, group_by, stmt->group);
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

Vector<String> exec_select(const Vector<String> &title, const Vector<String> &row,
                           const String &select) {
    Vector<String> new_row;

    // val cols = split_string(select, ',');
    // for (val &col: cols) {
    //     if (trim(col) == "*") {
    //         new_row.insert(new_row.end(), row.begin(), row.end());
    //         continue;
    //     }
    //
    //     val reg = Regex(R"(\s+(AS|as)\s+)");
    //     std::smatch match;
    //     using std::regex_search;
    //     val pos = regex_search(col, match, reg) ? match.position(0) : npos;
    //     val old_col = pos == npos ? trim(col) : trim(col.substr(0, pos));
    //     val idx = std::find(title.begin(), title.end(), old_col);
    //     val new_col = row[std::distance(title.begin(), idx)];
    //     new_row.push_back(new_col);
    // }
    return new_row;
}

bool exec_where(const Vector<String> &title, const Vector<String> &row, const String &where) {
    // if (where.empty()) {
    //     return true;
    // }
    //
    // val clauses = split_string_by_spaces(where);
    // if (clauses.size() < 3) {
    //     show_error("Unknown error.");
    // }
    // val &col = clauses[0];
    // val col_at = std::distance(title.begin(), std::find(title.begin(), title.end(), col));
    // if (col_at >= title.size()) {
    //     show_error("Column '" + col + "' not found.");
    // }
    // val &col_value = row[col_at];
    // val NOT = clauses.size() == 4;
    // val &type = clauses[clauses.size() - 2];
    // var pattern_str = clauses[clauses.size() - 1];
    //
    // if (type == "LIKE" || type == "like") {
    //     replace_all(pattern_str, "\\%", "@");
    //     replace_all(pattern_str, "%", ".*");
    //     replace_all(pattern_str, "@", "%");
    //
    //     replace_all(pattern_str, "\\_", "@");
    //     replace_all(pattern_str, "_", ".");
    //     replace_all(pattern_str, "@", "_");
    // } else if (type != "REG" && type != "reg") {
    //     show_error("Unknown error.");
    // }
    //
    // Regex reg = Regex(pattern_str);
    // val res = std::regex_match(col_value, reg);
    //
    // return NOT ^ res;
    return true;
}

Vector<String> handle_title(const Vector<String> &title, const String &select) {
    Vector<String> new_title;

    // val cols = split_string(select, ',');
    // for (val &col: cols) {
    //     if (trim(col) == "*") {
    //         new_title.insert(new_title.end(), title.begin(), title.end());
    //         continue;
    //     }
    //     val reg = Regex(R"(\s+(AS|as)\s+)");
    //     std::smatch match;
    //     using std::regex_search;
    //     val pos = regex_search(col, match, reg) ? match.position(0) : npos;
    //     if (pos != npos && pos >= col.size() - 4) {
    //         show_error("Syntax error near '" + col + "'.");
    //     }
    //     val old_col = pos == npos ? trim(col) : trim(col.substr(0, pos));
    //     val idx = std::find(title.begin(), title.end(), old_col);
    //     if (idx == title.end()) {
    //         show_error("Column '" + old_col + "' not found");
    //     }
    //     val alias = pos == npos ? "" : trim(col.substr(pos + match.str().size()));
    //     val new_col = alias.empty() ? old_col : alias;
    //
    //     for_each(new_title.begin(), new_title.end(), [&new_col](const String &s) {
    //         if (new_col == s) show_error("Duplicate column name '" + new_col + "'.");
    //     });
    //
    //     new_title.push_back(new_col);
    // }
    return new_title;
}

Vector<Pair<int, bool>> check_orders(const Vector<String> &data, const Vector<String> &orders) {
    Vector<Pair<int, bool>> sort_order;
    // for (val &order: orders) {
    //     val cols = split_string_by_spaces(trim(order));
    //     if (cols.size() > 2) {
    //         show_error("Syntax error near '" + order + "'.");
    //     }
    //     val &col = cols[0];
    //     val idx = std::distance(data.begin(), std::find(data.begin(), data.end(), col));
    //     if (idx >= data.size()) {
    //         show_error("Column '" + col + "' not found.");
    //     }
    //
    //     val direction = cols.size() == 2 ? cols[1] : "";
    //     val asc = direction.empty() || direction == "ASC" || direction == "asc";
    //
    //     sort_order.emplace_back(idx, asc);
    // }

    return sort_order;
}

void sort_data(Vector<Vector<String>> &data, const String &order) {
    // if (order.empty()) {
    //     return;
    // }
    // val orders = split_string(order, ',');
    // val sort_order = check_orders(data[0], orders);
    //
    // std::sort(data.begin() + 1, data.end(),
    //           [&sort_order](const Vector<String> &a, const Vector<String> &b) -> bool {
    //               var res = false;
    //               for (val &order: sort_order) {
    //                   val o = a[order.first].compare(b[order.first]);
    //                   if (o == 0) {
    //                       continue;
    //                   }
    //                   res = o > 0;
    //                   return order.second ^ res;
    //               }
    //               return false;
    //           });
}

Vector<Vector<String>> process_query(const Vector<Vector<String>> &input,
                                     const String &query) {

    // cut query
    std::smatch match;
    using std::regex_search;
    val limit_reg = Regex(R"(\s+(LIMIT|limit)\s+)");
    val order_reg = Regex(R"(\s+(ORDER|order)\s+(BY|by)\s+)");
    val where_reg = Regex(R"(\s+(WHERE|where)\s+)");

    var test = regex_search(query, match, limit_reg);
    val limits_pos = test ? match.position(0) : npos;
    val limits = trim(limits_pos != npos ? query.substr(limits_pos + match.str(0).size()) : "");
    var offset = 0, limit = 0;
    if (!limits.empty()) {
        val offset_pos = limits.find(',');
        offset = offset_pos != npos ? stoi(trim(limits.substr(0, offset_pos))) : 0;
        limit = stoi(trim(offset_pos != npos ? limits.substr(offset_pos + 1) : limits));

        // when specify limits, limit cannot be zero
        if (!limit) {
            show_warn("Zero limits will be ignored...");
        }
    }
    var rest = query.substr(0, limits_pos);

    val order_pos = regex_search(query, match, order_reg) ? match.position(0) : npos;
    val order = trim(order_pos != npos ? rest.substr(order_pos + match.str(0).size()) : "");
    rest = rest.substr(0, order_pos);

    val where_pos = regex_search(query, match, where_reg) ? match.position(0) : npos;
    val where = trim(where_pos != npos ? rest.substr(where_pos + match.str(0).size()) : "");
    val select = trim(rest.substr(0, where_pos).substr(7));
    // cut query end

    Vector<Vector<String>> output;
    val new_title = handle_title(input[0], select);
    output.push_back(new_title);
    for (var i = 1; i < input.size(); i++) {
        // 1. where
        if (exec_where(input[0], input[i], where)) {
            // 2. select
            var new_row = exec_select(input[0], input[i], select);
            output.push_back(new_row);
        }
    }
    // 3. order by
    sort_data(output, order);
    // 4. limit
    if (limit > 0) {
        // offset
        if (offset > 0) {
            if (offset < output.size()) {
                output.erase(output.begin() + 1, output.begin() + offset + 1);
            } else {
                output.clear();
            }
        }
        // limit
        if (limit + 1 < output.size()) {
            output.resize(limit + 1);
        }
    }

    return output;
}

void verify_query(const String &query) {
    val reg = Regex(
            R"((SELECT|select)\s+((([\w_][\w_\d]+)(\s+(AS|as)\s+([\w_][\w_\d]+))?)|\*)(\s*,\s*((([\w_][\w_\d]+)(\s+(AS|as)\s+([\w_][\w_\d]+))?)|\*))*(\s+(WHERE|where)\s+([\w_][\w_\d]+)(\s+(NOT|not))?\s+((LIKE|like)\s+((\\)?[%_]|[\d\w_])+|(REG|reg)\s+.+))?(\s+(ORDER|order)\s+(BY|by)\s+([\w_][\w_\d]+)(\s+(ASC|asc|DESC|desc))?(\s*,\s*([\w_][\w_\d]+)(\s+(ASC|asc|DESC|desc))?)*)?(\s+(LIMIT|limit)\s+(\d+)(\s*,\s*\d+)?)?)");
    if (!std::regex_match(trim(query), reg)) {
        show_error("Unrecognized query statement: '" + query + "'");
    }
}

Vector<Vector<String>> process_data(const ProgramOptions &options) {
    // init column names
    // val col_count = get_col_count(options);
    // Vector<String> col_names(col_count);
    // for (var i = 0; i < col_count; i++) {
    //     col_names[i] = "col" + to_string(i + 1);
    // }
    //
    // // prepare data to be processed
    // Vector<Vector<String>> output;
    // output.push_back(col_names);
    // prepare_data(options.data, output, options.delimiter);
    // if (!trim(options.query).empty()) {
    //     // Split the query into multiple queries
    //     val queries = split_string(options.query, '|');
    //
    //     for (val &query: queries) {
    //         verify_query(query);
    //         output = process_query(output, query);
    //     }
    // }
    //
    // return output;
    return {};
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

    return res;
}
