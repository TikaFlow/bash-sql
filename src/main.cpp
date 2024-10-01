//
// Created by tika on 4/21/24.
//

#include "main.h"

#define NO_LEN 5
#define COL_INIT_LEN 8
#define COL_PREFIX "__col_"

int main(int argc, char *argv[]) {
    // init_parser signal handler
    signal(SIGINT, [](int) { cout << endl << "Type 'exit' to exit." << flush; });

    // init_parser functions mapping
    init_funcs();
    // init_parser global variables
    options = parse_cmd_options(argc, argv);
    db = new Map<String, Pair<Schema *, Result *>>();

    import_data("std");
    if (!options->sql.empty()) {
        try {
            history.emplace_back(options->sql);
            handle_curd();
        } catch (std::exception &e) {
            cout << endl << e.what() << flush;
        }
    }
    if (options->interactive) {
        interactive();
    }

    return 0;
}

/**
 * Show error message.
 * @param error error message
 */
static void arg_error(const String &error) {
    show_error(error + "\nUse -h for help.");
}

/**
 * Show version message.
 */
static void show_version() {
    cout << "Bash-sql V" << VERSION << " by " << AUTHOR << endl;
}

/**
 * Show help message.
 */
static void show_help() {
    show_version();
    cout << APP_DESC "\n"
            "\n"
            "Usage: sql [OPTION] [QUERIES]\n"
            "-h, --help: Display this help and exit.\n"
            "-v, --version: Output version information and exit.\n"
            "-t, --title: First row is table title.\n"
            "-l, --line-no: Print line number.\n"
            "-i, --interactive: Interactive mode.\n"
            "-f, --file=FILE: Read data from FILE.\n"
            "-d, --delimiter=DELIMITER: Use DELIMITER as field delimiter.\n"
            "-c, --columns=COLUMNS: Use COLUMNS as number of columns.\n"
         << endl;
}

/**
 * parse command line options
 * @param argc number of arguments
 * @param argv arguments
 * @return program options
 */
ProgramOptions *parse_cmd_options(int argc, char *argv[]) {
    val opt = new ProgramOptions{false, false, false, "", '\0', 0, ""};

    var help = false;
    var version = false;
    var data = false;
    var file = false;
    var filename = String();

    static option long_options[] = {
            {"help",        no_argument,       null, 'h'},
            {"version",     no_argument,       null, 'v'},
            {"title",       no_argument,       null, 't'},
            {"line-no",     no_argument,       null, 'l'},
            {"interactive", no_argument,       null, 'i'},
            {"file",        required_argument, null, 'f'},
            {"delimiter",   required_argument, null, 'd'},
            {"columns",     required_argument, null, 'c'},
            {null, 0,                          null, 0}
    };

    // check if there is data from stdin
    if (isatty(STDIN_FILENO) == 0) {
        data = true;
        String line;
        while (getline(cin, line)) {
            opt->data += line + "\n";
        }
        if (!opt->data.empty()) {
            opt->data.pop_back();
        }
    }

    int c;
    while ((c = getopt_long(argc, argv, "hvtlid:f:c:", long_options, null)) != -1) {
        switch (c) {
            case 'h':
                help = true;
                break;
            case 'v':
                version = true;
                break;
            case 't':
                opt->title = true;
                break;
            case 'l':
                opt->line_no = true;
                break;
            case 'i':
                opt->interactive = true;
                break;
            case 'f':
                // check file exists
                if (access(optarg, F_OK)) {
                    arg_error("File not found: " + String(optarg));
                }
                file = true;
                filename = optarg;
                break;
            case 'd':
                opt->delimiter = optarg[0];
                break;
            case 'c':
                opt->columns = stoi(optarg);
                if (opt->columns < 0) {
                    arg_error("Invalid column number: " + String(optarg));
                }
                break;
            case '?':
                arg_error("Unknown option.");
                break;
            default:
                arg_error("Unknown error.");
                break;
        }
    }

    // sql string
    for (var i = optind; i < argc; ++i) {
        opt->sql += argv[i];
        if (i < argc - 1) {
            opt->sql += " ";
        }
    }

    opt->sql = trim(opt->sql);
    // check mutually exclusive options
    if (help || version) {
        if (opt->title || data || file || opt->delimiter || opt->columns || !opt->sql.empty()) {
            if (help) {
                arg_error("Help option cannot be used with other options.");
            }
            if (version) {
                arg_error("Version option cannot be used with other options.");
            }
        }
    } else {
        if (data && file) {
            arg_error("File and Standard Input cannot be specified simultaneously.");
        }
    }

    // print help info
    if (help) {
        show_help();

        exit(0);
    }

    // print version info
    if (version) {
        show_version();
        exit(0);
    }

    // get data string
    if (file) {
        opt->data = read_file_to_string(filename);
    }

    return opt;
}

/**
 * Handle CURD operations
 * @param options CURD options
 */
void handle_curd() {
    val tokens = lex();
    if (tokens->empty()) {
        return;
    }

    var stmt = parse(tokens);
    val result = process(stmt);

    bool old_config;
    val free_stmt = [&stmt]() {
        stmt.release();
    };

    switch (stmt.type) {
        case S_CREATE:
            print_show(result, stmt.name, free_stmt);
            break;
        case S_INSERT:
            print_show(result, "insert", free_stmt);
            break;
        case S_UPDATE:
            print_show(result, "update", free_stmt);
            break;
        case S_SELECT:
            print_read(result, stmt.stmt_select->select, free_stmt);
            break;
        case S_DROP:
            print_show(result, "drop", free_stmt);
            break;
        case S_DELETE:
            print_show(result, "delete", free_stmt);
            break;
        case S_DESCRIBE:
            print_show(result, stmt.name);
            break;
        case S_SHOW:
            print_show(result, "tables");
            break;
        case S_HISTORY:
            old_config = options->line_no;
            options->line_no = true;
            print_show(result, "history", [old_config]() {
                options->line_no = old_config;
            });
            break;
        default:
            break;
    }
}

/**
 * Read and execute SQL commands in interactive mode
 * @param options CURD options
 */
void interactive() {

    while (true) {
        cout << endl << "(sql)> " << flush;

        std::getline(std::cin, options->sql);
        options->sql = trim(options->sql);

        if (options->sql.empty()) {
            continue;
        }

        val &cmd = options->sql;
        if (cmd == "exit" || cmd == "exit()" || cmd == "exit;" || cmd == "exit();") {
            cout << "Bye!" << endl;
            break;
        }

        try {
            history.emplace_back(options->sql);
            handle_curd();
        } catch (std::exception &e) {
            cout << endl << e.what() << flush;
        }
    }
}

/**
 * prepare data from string
 * @param data data string
 * @return data of table format
 */
void import_data(const String &table) {
    if (options->data.empty()) {
        return;
    }

    var lines = split_string(options->data, '\n');
    // data has been trimmed so that the first line won't be empty

    val res = new Result();
    for (int r = options->title; r < lines->size(); r++) {
        val line = lines->at(r);
        val row = new Row();
        var cols = options->delimiter
                   ? split_string(line, options->delimiter)
                   : split_string_by_spaces(line);

        // correct columns count
        if (!options->columns) {
            options->columns = (int) cols->size();
        }

        val size = cols->size();
        if (size > options->columns) {
            for (var i = options->columns; i < size; i++) {
                cols->at(options->columns - 1) += "|" + cols->at(i);
            }
        }
        cols->resize(options->columns);

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
    val schema = new Schema();
    if (options->title) {
        val line = lines->at(0);
        var cols = options->delimiter
                   ? split_string(line, options->delimiter)
                   : split_string_by_spaces(line);

        val size = cols->size();
        if (size > options->columns) {
            for (var i = options->columns; i < size; i++) {
                cols->at(options->columns - 1) += "_" + cols->at(i);
            }
        }
        cols->resize(options->columns);

        for (val &col: *cols) {
            schema->push_back({col, D_STRING});
        }
    } else {
        for (var i = 0; i < options->columns; ++i) {
            schema->push_back({"col" + std::to_string(i + 1), D_STRING});
        }
    }

    db->insert({to_lower(table), {schema, res}});
}

void export_data(const String &table, const String &file, bool with_title, bool with_line_no, char deli) {
    val table_name = to_lower(table);
    if (!db->count(table_name)) {
        show_error("Table not found: " + table_name);
    }

    val tbl = db->at(table_name);
    val schema = tbl.first;
    val res = tbl.second;
    val columns = res->at(0)->size();
    std::ostringstream oss;

    var col_no = 1;
    // title
    if (with_title) {
        if (with_line_no) {
            oss << "__line_no" << deli;
        }
        for (val &col: *schema) {
            oss << col.first;
            if (col_no < columns) {
                oss << deli;
            }
            col_no++;
        }
        oss << endl;
    }

    // data
    var line_no = 1;
    for (val &row: *res) {
        col_no = 1;
        if (with_line_no) {
            oss << line_no << deli;
        }
        for (val &col: *row) {
            oss << col->to_string();
            if (col_no < columns) {
                oss << deli;
            }
            col_no++;
        }
        oss << endl;
        line_no++;
    }

    save_string_to_file(file, oss.str());
}

/**
 * print dashes line
 * @param width column width
 */
static void print_dashes(int width) {
    if (options->line_no) {
        cout << "+" << setw(NO_LEN) << std::right << setfill('-') << "" << "-";
    }
    cout << "+-" << setw(width) << std::right << setfill('-') << "" << "-";
    cout << "+" << endl;
}

/**
 * print dashes line
 * @param cw column width
 * @param count column count
 */
static void print_dashes(Vector<int> &cw, size_t count) {
    if (options->line_no) {
        cout << "+" << setw(NO_LEN) << std::right << setfill('-') << "" << "-";
    }
    for (var i = 0; i < count; i++) {
        cout << "+-" << setw(cw.at(i)) << std::right << setfill('-') << "" << "-";
    }
    cout << "+" << endl;
}

/**
 * print result of one column
 * @param data data
 * @param title title
 */
void print_show(Result *data, const String &title, const Function<void()> &func) {
    // column width
    var cw = (int) title.length();
    for (val &row: *data) {
        val len = (int) row->at(0)->text.length();
        if (len > cw) {
            cw = len;
        }
    }

    // title
    print_dashes(cw);
    if (options->line_no) {
        cout << "| " << setw(NO_LEN) << std::left << setfill(' ') << "  No ";
    }
    cout << "| " << setw(cw) << std::left << setfill(' ') << title << " ";
    cout << "|" << endl;
    print_dashes(cw);

    // data
    var line_no = 1;
    for (val &row: *data) {
        val res = row->at(0);
        if (options->line_no) {
            cout << "| " << setw(NO_LEN) << std::left << setfill(' ') << line_no++;
        }
        cout << "| " << setw(cw) << std::left << setfill(' ') << res->to_string() << " ";
        cout << "|" << endl;
    }
    print_dashes(cw);

    if (func) {
        func();
    }
    throw std::runtime_error(OK_MSG);
}

/**
 * print result of select query
 * @param data data
 * @param select select nodes
 */
void print_read(Result *data, Vector<SelectNode> *select, const Function<void()> &func) {
    // column width
    var cw = Vector<int>();
    for (var i = 0; i < select->size(); ++i) {
        cw.push_back(COL_INIT_LEN);
        if (cw.at(i) < select->at(i).as.length()) {
            cw.at(i) = (int) select->at(i).as.length();
        }
    }
    for (val &row: *data) {
        for (var i = 0; i < row->size(); ++i) {
            val cell_len = row->at(i)->to_string().length();
            if (cw.at(i) < cell_len) {
                cw.at(i) = (int) cell_len;
            }
        }
    }

    // title
    print_dashes(cw, select->size());
    if (options->line_no) {
        cout << "| " << setw(NO_LEN) << std::left << setfill(' ') << "  No ";
    }
    var coli = 1;
    for (var i = 0; i < select->size(); ++i) {
        val as = select->at(i).as;
        cout << "| " << setw(cw.at(i)) << std::left << setfill(' ') <<
             (as.empty() ? COL_PREFIX + to_string(coli) : as) << " ";
        coli++;
    }
    cout << "|" << endl;
    print_dashes(cw, select->size());

    // data
    var line_no = 1;
    for (val &row: *data) {
        if (options->line_no) {
            cout << "| " << setw(NO_LEN) << std::left << setfill(' ') << line_no++;
        }

        for (var i = 0; i < row->size(); ++i) {
            cout << "| " << setw(cw.at(i)) << std::left << setfill(' ')
                 << row->at(i)->to_string() << " ";
        }
        cout << "|" << endl;
    }
    print_dashes(cw, select->size());

    if (func) {
        func();
    }
    throw std::runtime_error(OK_MSG);
}

ProgramOptions *options;
Map<String, Pair<Schema *, Result *>> *db;
Vector<String> history;
