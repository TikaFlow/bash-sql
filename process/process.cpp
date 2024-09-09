//
// Created by tika on 5/2/24.
//

#include "process.h"

void prepare_data(const String &data, Vector<Vector<String>> &output, const char d) {
    val col_count = output[0].size();
    var rows = split_string(data, '\n');
    for (var &row: rows) {
        var cols = d == 0 ? split_string_by_spaces(row) : split_string(row, d);
        if (cols.size() > col_count) {
            for (var i = col_count; i < cols.size(); i++) {
                cols[col_count - 1] += " " + cols[i];
            }
        }
        cols.resize(col_count);
        output.push_back(cols);
    }
}

int get_col_count(const ProgramOptions &options) {
    if (options.columns > 0) {
        return options.columns;
    }

    val first_line = split_string(options.data, '\n')[0];
    val col_count = options.delimiter == 0
                    ? split_string_by_spaces(first_line).size()
                    : split_string(first_line, options.delimiter).size();
    return (int) col_count;
}

Vector<String> exec_select(const Vector<String> &title, const Vector<String> &row,
                           const String &select) {
    Vector<String> new_row;

    val cols = split_string(select, ',');
    for (val &col: cols) {
        if (trim(col) == "*") {
            new_row.insert(new_row.end(), row.begin(), row.end());
            continue;
        }

        val reg = Regex(R"(\s+(AS|as)\s+)");
        std::smatch match;
        using std::regex_search;
        val pos = regex_search(col, match, reg) ? match.position(0) : npos;
        val old_col = pos == npos ? trim(col) : trim(col.substr(0, pos));
        val idx = std::find(title.begin(), title.end(), old_col);
        val new_col = row[std::distance(title.begin(), idx)];
        new_row.push_back(new_col);
    }
    return new_row;
}

bool exec_where(const Vector<String> &title, const Vector<String> &row, const String &where) {
    if (where.empty()) {
        return true;
    }

    val clauses = split_string_by_spaces(where);
    if (clauses.size() < 3) {
        show_error("Unknown error.");
    }
    val &col = clauses[0];
    val col_at = std::distance(title.begin(), std::find(title.begin(), title.end(), col));
    if (col_at >= title.size()) {
        show_error("Column '" + col + "' not found.");
    }
    val &col_value = row[col_at];
    val NOT = clauses.size() == 4;
    val &type = clauses[clauses.size() - 2];
    var pattern_str = clauses[clauses.size() - 1];

    if (type == "LIKE" || type == "like") {
        replaceAll(pattern_str, "\\%", "@");
        replaceAll(pattern_str, "%", ".*");
        replaceAll(pattern_str, "@", "%");

        replaceAll(pattern_str, "\\_", "@");
        replaceAll(pattern_str, "_", ".");
        replaceAll(pattern_str, "@", "_");
    } else if (type != "REG" && type != "reg") {
        show_error("Unknown error.");
    }

    Regex reg = Regex(pattern_str);
    val res = std::regex_match(col_value, reg);

    return NOT ^ res;
}

Vector<String> handle_title(const Vector<String> &title, const String &select) {
    Vector<String> new_title;

    val cols = split_string(select, ',');
    for (val &col: cols) {
        if (trim(col) == "*") {
            new_title.insert(new_title.end(), title.begin(), title.end());
            continue;
        }
        val reg = Regex(R"(\s+(AS|as)\s+)");
        std::smatch match;
        using std::regex_search;
        val pos = regex_search(col, match, reg) ? match.position(0) : npos;
        if (pos != npos && pos >= col.size() - 4) {
            show_error("Syntax error near '" + col + "'.");
        }
        val old_col = pos == npos ? trim(col) : trim(col.substr(0, pos));
        val idx = std::find(title.begin(), title.end(), old_col);
        if (idx == title.end()) {
            show_error("Column '" + old_col + "' not found");
        }
        val alias = pos == npos ? "" : trim(col.substr(pos + match.str().size()));
        val new_col = alias.empty() ? old_col : alias;

        for_each(new_title.begin(), new_title.end(), [&new_col](const String &s) {
            if (new_col == s) show_error("Duplicate column name '" + new_col + "'.");
        });

        new_title.push_back(new_col);
    }
    return new_title;
}

Vector<Pair<int, bool>> check_orders(const Vector<String> &data, const Vector<String> &orders) {
    Vector<Pair<int, bool>> sort_order;
    for (val &order: orders) {
        val cols = split_string_by_spaces(trim(order));
        if (cols.size() > 2) {
            show_error("Syntax error near '" + order + "'.");
        }
        val &col = cols[0];
        val idx = std::distance(data.begin(), std::find(data.begin(), data.end(), col));
        if (idx >= data.size()) {
            show_error("Column '" + col + "' not found.");
        }

        val direction = cols.size() == 2 ? cols[1] : "";
        val asc = direction.empty() || direction == "ASC" || direction == "asc";

        sort_order.emplace_back(idx, asc);
    }

    return sort_order;
}

void sort_data(Vector<Vector<String>> &data, const String &order) {
    if (order.empty()) {
        return;
    }
    val orders = split_string(order, ',');
    val sort_order = check_orders(data[0], orders);

    std::sort(data.begin() + 1, data.end(),
              [&sort_order](const Vector<String> &a, const Vector<String> &b) -> bool {
                  var res = false;
                  for (val &order: sort_order) {
                      val o = a[order.first].compare(b[order.first]);
                      if (o == 0) {
                          continue;
                      }
                      res = o > 0;
                      return order.second ^ res;
                  }
                  return false;
              });
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
    val col_count = get_col_count(options);
    Vector<String> col_names(col_count);
    for (var i = 0; i < col_count; i++) {
        col_names[i] = "col" + to_string(i + 1);
    }

    // prepare data to be processed
    Vector<Vector<String>> output;
    output.push_back(col_names);
    prepare_data(options.data, output, options.delimiter);
    if (!trim(options.query).empty()) {
        // Split the query into multiple queries
        val queries = split_string(options.query, '|');

        for (val &query: queries) {
            verify_query(query);
            output = process_query(output, query);
        }
    }

    return output;
}

Vector<Line> *apply(SelectStatement *query, const String &data, int col_count, char d) {
    if (!query) {
        return null;
    }
    if (data.empty() && !query->tableless()) {
        show_error("Data is empty, but query is not tableless");
    }

    val res = new Vector<Line>();

    // TODO : implement

    return res;
}
