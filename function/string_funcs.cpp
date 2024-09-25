//
// Created by tika on 24-9-16.
//

#include "funcs.h"

extern Cell *tk_conv(Row *row);

/**
 * aux function for bin and oct
 * @param row params
 * @param func_name who calls this function
 * @param base base
 * @return converted string cell
 */
static Cell *bin_oct_aux(Row *row, const String &func_name, int base) {
    check_arg_nums(row, func_name, 1);

    val cell = row->at(0);
    var from = Cell(D_INTEGER, 10);
    var to = Cell(D_INTEGER, base);
    val new_row = new Row();
    new_row->push_back(cell);
    new_row->push_back(&from);
    new_row->push_back(&to);

    return tk_conv(new_row);
}

Cell *concat_aux(Row *row, const String &func_name, const String &sep = "", int start = 0);

/**
 * aux function for concat/concat_ws/group_concat
 * @param row params
 * @param func_name who calls this function
 * @param sep separator
 * @param start start index
 * @return concatenated string cell
 */
Cell *concat_aux(Row *row, const String &func_name, const String &sep, int start) {
    check_arg_nums(row, func_name, start + 1, PARAM_MAX);

    var str = String();
    for (var i = start; i < row->size(); ++i) {
        val cell = row->at(i);
        if (check_null(cell)) {
            if (start) {
                continue;
            }
            return new Cell();
        }
        str += cell->to_string();

        if (i == row->size() - 1) {
            break;
        }
        str += sep;
    }

    return new Cell(str);
}

/**
 * aux function for instr and locate
 * @param row params
 * @param func_name who calls this function
 * @param pos start index
 * @return index cell
 */
static Cell *find_str_aux(Row *row, const String &func_name, int pos = 1) {
    check_arg_nums(row, func_name, 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val str = row->at(func_name != "instr")->to_string();
    val substr = row->at(func_name == "instr")->to_string();

    if (pos < 0 || pos > str.length()) {
        return new Cell(D_INTEGER, 0);
    }

    return new Cell(D_INTEGER, str.find(substr, pos - 1) + 1);
}

static Cell *tk_ascii(Row *row) {
    check_arg_nums(row, "ascii", 1);

    val cell = row->at(0);
    if (check_null(cell)) {
        return new Cell();
    }

    val str = cell->to_string();
    if (str.empty()) {
        return new Cell(D_INTEGER, 0);
    }

    return new Cell(D_INTEGER, (int) cell->to_string().at(0));
}

static Cell *tk_bin(Row *row) {
    return bin_oct_aux(row, "bin", 2);
}

static Cell *tk_char(Row *row) {
    check_arg_nums(row, "char", 1, PARAM_MAX);

    var res = String();
    for (val &cell: *row) {
        if (check_null(cell)) {
            continue;
        }

        val num = strtod(cell->to_string().c_str(), null);
        res += (char) num;
    }

    return new Cell(res);
}

static Cell *tk_concat(Row *row) {
    return concat_aux(row, "concat");
}

static Cell *tk_concat_ws(Row *row) {
    val sep = row->at(0);
    if (check_null(sep)) {
        return new Cell();
    }

    return concat_aux(row, "concat_ws", sep->to_string(), 1);
}

static Cell *tk_elt(Row *row) {
    check_arg_nums(row, "elt", 2, PARAM_MAX);

    val cell = row->at(0);
    if (check_null(cell) || cell->type != D_INTEGER
        || cell->number < 1 || cell->number >= row->size()) {
        return new Cell();
    }

    return row->at((int) cell->number);
}

static Cell *tk_field(Row *row) {
    check_arg_nums(row, "field", 2, PARAM_MAX);

    val cell = row->at(0);
    val res = new Cell(D_INTEGER, 0);
    if (check_null(cell)) {
        return res;
    }

    for (var i = 1; i < row->size(); ++i) {
        val cur = row->at(i);
        if (check_equal(cell, cur)) {
            res->number = i;
            break;
        }
    }

    return res;
}

static Cell *tk_hex(Row *row) {
    check_arg_nums(row, "hex", 1);

    val cell = row->at(0);
    if (cell->type != D_INTEGER && cell->type != D_STRING) {
        show_error("hex: argument must be a integer or string");
    }

    std::ostringstream oss;
    if (cell->type == D_INTEGER) {
        oss << std::uppercase << std::hex << (int) cell->number;
    } else {
        for (var c: cell->text) {
            oss << std::uppercase << std::hex << setw(2) << setfill('0') << (int) c;
        }
    }

    return new Cell(oss.str());
}

static Cell *tk_insert(Row *row) {
    check_arg_nums(row, "insert", 4);

    val c_str = row->at(0);
    val c_start = row->at(1);
    val c_len = row->at(2);
    val c_ins = row->at(3);

    if (check_null(c_str) || check_null(c_start) || check_null(c_len) || check_null(c_ins)) {
        return new Cell();
    }

    var str = c_str->to_string();
    var start = 0;
    if (c_start->type == D_INTEGER) {
        start = (int) c_start->number;
    }
    var len = 0;
    if (c_len->type == D_INTEGER) {
        len = (int) c_len->number;
    }
    val ins = c_ins->to_string();

    if (start > 1 && start < str.length()) {
        if (len < 0 || start + len > str.length()) {
            str = str.substr(0, start - 1) + ins;
        } else {
            str = str.substr(0, start - 1) + ins + str.substr(start + len - 1);
        }
    }

    return new Cell(str);
}

static Cell *tk_instr(Row *row) {
    return find_str_aux(row, "instr");
}

static Cell *tk_lower(Row *row);

static Cell *tk_lcase(Row *row) {
    check_arg_nums(row, "lcase", 1);
    return tk_lower(row);
}

static Cell *tk_left(Row *row) {
    check_arg_nums(row, "left", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    var len = 0;
    if (row->at(1)->type == D_INTEGER) {
        len = (int) row->at(1)->number;
    }

    if (len < 0 || len > str.length()) {
        return new Cell(String());
    }

    return new Cell(str.substr(0, len));
}

static Cell *tk_length(Row *row) {
    check_arg_nums(row, "length", 1);

    val cell = row->at(0);
    if (check_null(cell)) {
        return new Cell(D_INTEGER, 0);
    }

    return new Cell(D_INTEGER, cell->to_string().length());
}

static Cell *tk_locate(Row *row) {
    check_arg_nums(row, "locate", 2, 3);

    if (row->size() == 2) {
        return find_str_aux(row, "locate");
    }

    if (check_null(row->at(2))) {
        return new Cell();
    }

    val start = row->at(2);
    var pos = 0;
    if (start->type == D_INTEGER) {
        pos = (int) start->number;
    }

    return find_str_aux(row, "locate", pos);
}

static Cell *tk_lower(Row *row) {
    check_arg_nums(row, "lower", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    return new Cell(to_lower(str));
}

static Cell *tk_lpad(Row *row) {
    check_arg_nums(row, "lpad", 3);

    if (check_null(row->at(0)) || check_null(row->at(1)) || check_null(row->at(2))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    var len = 0;
    if (row->at(1)->type == D_INTEGER) {
        len = (int) row->at(1)->number;
    }
    val pad = row->at(2)->to_string();

    var res = String();
    if (len <= str.length()) {
        res = str.substr(0, len);
    } else {
        val diff = len - str.length();
        while (res.length() < diff) {
            res += pad;
        }
        res = res.substr(0, diff) + str;
    }

    return new Cell(res);
}

static Cell *tk_ltrim(Row *row) {
    check_arg_nums(row, "ltrim", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    return new Cell(ltrim(str));
}

static Cell *tk_substring(Row *row);

static Cell *tk_mid(Row *row) {
    check_arg_nums(row, "mid", 2, 3);
    return tk_substring(row);
}

static Cell *tk_oct(Row *row) {
    return bin_oct_aux(row, "oct", 8);
}

static Cell *tk_position(Row *row) {
    check_arg_nums(row, "position", 2);
    return find_str_aux(row, "position");
}

static Cell *tk_repeat(Row *row) {
    check_arg_nums(row, "repeat", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    var str = row->at(0)->to_string();
    var times = 0;
    if (row->at(1)->type == D_INTEGER) {
        times = (int) row->at(1)->number;
    }

    if (times <= 0) {
        return new Cell(String());
    }

    var res = String();
    while (times) {
        if (times & 1) {
            res += str;
        }
        str += str;
        times >>= 1;
    }

    return new Cell(res);
}

static Cell *tk_replace(Row *row) {
    check_arg_nums(row, "replace", 3);

    if (check_null(row->at(0)) || check_null(row->at(1)) || check_null(row->at(2))) {
        return new Cell();
    }

    var str = row->at(0)->to_string();
    val from_str = row->at(1)->to_string();
    val to_str = row->at(2)->to_string();

    return new Cell(replace_all(str, from_str, to_str));
}

static Cell *tk_reverse(Row *row) {
    check_arg_nums(row, "reverse", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    char arr[str.length() + 1];
    for (int i = 0; i < str.length(); ++i) {
        arr[i] = str[str.length() - i - 1];
    }
    arr[str.length()] = '\0';

    return new Cell(String(arr));
}

static Cell *tk_right(Row *row) {
    check_arg_nums(row, "right", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    var len = 0;
    if (row->at(1)->type == D_INTEGER) {
        len = (int) row->at(1)->number;
    }

    if (len < 0 || len > str.length()) {
        return new Cell(String());
    }

    return new Cell(str.substr(str.length() - len));
}

static Cell *tk_rpad(Row *row) {
    check_arg_nums(row, "rpad", 3);

    if (check_null(row->at(0)) || check_null(row->at(1)) || check_null(row->at(2))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    var len = 0;
    if (row->at(1)->type == D_INTEGER) {
        len = (int) row->at(1)->number;
    }
    val pad = row->at(2)->to_string();

    var res = String();
    if (len <= str.length()) {
        res = str.substr(0, len);
    } else {
        val diff = len - str.length();
        while (res.length() < diff) {
            res += pad;
        }
        res = str + res.substr(0, diff);
    }

    return new Cell(res);
}

static Cell *tk_rtrim(Row *row) {
    check_arg_nums(row, "rtrim", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    return new Cell(rtrim(str));
}

static Cell *tk_space(Row *row) {
    check_arg_nums(row, "space", 1);

    var space_cell = Cell(String(" "));
    val new_row = new Row();
    new_row->push_back(&space_cell);
    new_row->push_back(row->at(0));

    return tk_repeat(new_row);
}

static Cell *tk_strcmp(Row *row) {
    check_arg_nums(row, "strcmp", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val left = row->at(0)->to_string();
    val right = row->at(1)->to_string();

    return new Cell(D_INTEGER, left.compare(right));
}

static Cell *tk_substr(Row *row) {
    check_arg_nums(row, "substr", 2, 3);
    return tk_substring(row);
}

static Cell *tk_substring(Row *row) {
    check_arg_nums(row, "substring", 2, 3);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }
    if (row->size() == 3 && check_null(row->at(2))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    var pos = 0;
    if (row->at(1)->type == D_INTEGER) {
        pos = (int) row->at(1)->number;
    }
    if (pos == 0) {
        return new Cell(String());
    } else if (pos < 0) {
        pos += str.length();
    } else {
        pos -= 1;
    }

    if (row->size() == 3) {
        var len = 0;
        if (row->at(2)->type == D_INTEGER) {
            len = (int) row->at(2)->number;
        }
        if (len < 1) {
            return new Cell(String());
        }
        return new Cell(str.substr(pos, len));
    } else {
        return new Cell(str.substr(pos));
    }
}

static Cell *tk_substring_index(Row *row) {
    check_arg_nums(row, "substring_index", 3);

    if (check_null(row->at(0)) || check_null(row->at(1)) || check_null(row->at(2))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    val delim = row->at(1)->to_string();
    var count = 0;
    if (row->at(2)->type == D_INTEGER) {
        count = (int) row->at(2)->number;
    }

    if (count == 0 || delim.empty()) {
        return new Cell(String());
    }

    var map = Map<int, String::size_type>();
    String::size_type pos = 0;
    while (true) {
        pos = str.find(delim, pos);
        if (pos == npos) {
            break;
        }
        map.insert({map.size() + 1, pos});
        pos += delim.length() + 1;
    }

    var idx = count;
    if (idx < 0) {
        idx += map.size() + 1;
    }

    if (idx < 1 || idx > map.size()) {
        return new Cell(str);
    }

    if (count < 0) {
        return new Cell(str.substr(map.at(idx) + delim.length()));
    } else {
        return new Cell(str.substr(0, map.at(idx)));
    }
}

static Cell *tk_trim(Row *row) {
    check_arg_nums(row, "trim", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    return new Cell(trim(str));
}

static Cell *tk_upper(Row *row);

static Cell *tk_ucase(Row *row) {
    check_arg_nums(row, "ucase", 1);
    return tk_upper(row);
}

static Cell *tk_unhex(Row *row) {
    check_arg_nums(row, "unhex", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    var str = row->at(0)->to_string();
    if (!all_of(str.begin(), str.end(), [](char c) { return isxdigit(c); })) {
        return new Cell();
    }

    if (str.size() % 2) {
        str = "0" + str;
    }

    val dict = String("0123456789abcdef");
    char arr[str.size() / 2 + 1];
    for (int i = 0; i < str.size(); i += 2) {
        val high = dict.find(tolower(str.at(i)));
        val low = dict.find(tolower(str.at(i + 1)));
        arr[i / 2] = (char) ((high << 4) | low);
    }
    arr[str.size() / 2] = '\0';

    return new Cell(String(arr));
}

static Cell *tk_upper(Row *row) {
    check_arg_nums(row, "upper", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->to_string();
    return new Cell(to_upper(str));
}

void init_string_funcs() {
    ADD_NORMAL(ascii, D_INTEGER);
    ADD_NORMAL(bin, D_STRING);
    ADD_NORMAL(char, D_STRING);
    ADD_NORMAL(concat, D_STRING);
    ADD_NORMAL(concat_ws, D_STRING);
    ADD_NORMAL(elt, D_STRING);
    ADD_NORMAL(field, D_INTEGER);
    ADD_NORMAL(hex, D_STRING);
    ADD_NORMAL(insert, D_STRING);
    ADD_NORMAL(instr, D_INTEGER);
    ADD_NORMAL(lcase, D_STRING);
    ADD_NORMAL(left, D_STRING);
    ADD_NORMAL(length, D_STRING);
    ADD_NORMAL(locate, D_INTEGER);
    ADD_NORMAL(lower, D_STRING);
    ADD_NORMAL(lpad, D_STRING);
    ADD_NORMAL(ltrim, D_STRING);
    ADD_NORMAL(mid, D_STRING);
    ADD_NORMAL(oct, D_STRING);
    ADD_NORMAL(position, D_INTEGER);
    ADD_NORMAL(repeat, D_STRING);
    ADD_NORMAL(replace, D_STRING);
    ADD_NORMAL(reverse, D_STRING);
    ADD_NORMAL(right, D_STRING);
    ADD_NORMAL(rpad, D_STRING);
    ADD_NORMAL(rtrim, D_STRING);
    ADD_NORMAL(space, D_STRING);
    ADD_NORMAL(strcmp, D_STRING);
    ADD_NORMAL(substr, D_STRING);
    ADD_NORMAL(substring, D_STRING);
    ADD_NORMAL(substring_index, D_STRING);
    ADD_NORMAL(trim, D_STRING);
    ADD_NORMAL(ucase, D_STRING);
    ADD_NORMAL(unhex, D_STRING);
    ADD_NORMAL(upper, D_STRING);
}
