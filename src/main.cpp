//
// Created by tika on 4/21/24.
//

#include "main.h"

#define NO_LEN 5
#define COL_INIT_LEN 8
#define COL_PREFIX "__col_"
#define ERROR_MSG "1 ERROR."
#define OK_MSG "OK."

int main(int argc, char *argv[]) {
    // init_parser signal handler
    signal(SIGINT, [](int) { cout << endl << "Type 'exit' to exit." << flush; });

    // init_parser functions mapping
    init_funcs();
    // init_parser global variables
    options = parse_cmd_options(argc, argv);
    db = new Map<String, Pair<Schema *, Result *>>();

    prepare_data("std");
    if (options->interactive) {
        interactive();
    } else {
        handle_curd();
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
    cout << "Using SQL-like languages to process lightweight data in bash.\n"
            "\n"
            "Usage: sql [OPTION] [QUERIES]\n"
            "\n"
            "Options:\n"
            "  -h, --help                   display this help and exit.\n"
            "  -v, --version                output version information and exit.\n"
            "  -t, --title                  print table title.\n"
            "  -l, --line-no                print line number.\n"
            "  -f, --file=FILE              read data from FILE.\n"
            "  -d, --delimiter=DELIMITER    use DELIMITER as field delimiter.\n"
            "  -c, --columns=COLUMNS        use COLUMNS as number of columns.\n"
            "\n"
            "Queries:\n"
            "  QUERY [ | QUERY2 [...] ]\n"
            "       each query is a sql-like select statement separated by `|`:\n"
            "\n"
            "       select COLUMNS [WHERE] [ORDER BY] [LIMITS]\n"
            "\n"
            "       keywords are case-insensitive.\n"
            "  COLUMNS      list columns to select.\n"
            "       use '*' to select all columns.\n"
            "       default column names are col1, col2, ...\n"
            "       aliasing column by using 'as'.\n"
            "  WHERE        filter rows with like clause or reg clause.\n"
            "       use 'column like pattern' or 'column reg pattern' to filter.\n"
            "       add 'not' before 'like' or 'reg' to reverse.\n"
            "       when 'like', % and _ are supported just like in mysql, and escape them by \\.\n"
            "       when 'reg', pattern should be a regular expression.\n"
            "       pattern needs no quotes.\n"
            "  ORDER BY     reorder data after selecting.\n"
            "       specify one or more columns to order by.\n"
            "       use 'asc' for ascending, 'desc' for descending.\n"
            "       'asc' can be omitted.\n"
            "  LIMITS       limit output lines.\n"
            "       either 'limit lines' or 'limit offset, lines' are supported.\n"
            "\n"
            "Example:\n"
            "  ps -aux | sql -tlc11 \"select col1 as user, col2 as pid, col9 as start, col11 as command \\\n"
            "  where col2 not like PID | select * order by start desc limit 10\"\n" << endl;
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
        show_version();

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

    val query = parse(tokens);

    val result = process(query);

    print_data(result, query.stmt_r->select);
}

/**
 * Read and execute SQL commands in interactive mode
 * @param options CURD options
 */
void interactive() {
    // var tables = Vector<String>();

    while (true) {
        cout << endl << " (sql) > " << flush;

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
            handle_curd();
        } catch (...) {
            cout << endl << ERROR_MSG << flush;
        }
    }
}

/**
 * prepare data from string
 * @param data data string
 * @return data of table format
 */
void prepare_data(const String &table) {
    var lines = split_string(options->data, '\n');

    if (lines->empty()) {
        return;
    }
    // data has been trimmed so that the first line won't be empty

    val res = new Result();
    for (var &line: *lines) {
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
    for (var i = 0; i < options->columns; ++i) {
        schema->push_back({"col" + std::to_string(i + 1), D_STRING});
    }

    db->insert({table, {schema, res}});
}

/**
 * Print data
 * @param data data
 * @param select select nodes
 */
void print_data(Result *data, Vector<SelectNode> *select) {
    // column width
    var cw = Vector<int>();
    for (val &row: *data) {
        for (var i = 0; i < row->size(); ++i) {
            if (cw.size() <= i) {
                // the first row
                cw.push_back(COL_INIT_LEN);
            }

            val cell_len = row->at(i)->to_string().length();
            if (cw.at(i) < cell_len) {
                cw.at(i) = (int) cell_len;
            }
        }
    }

    if (options->title) {
        // title
        if (options->line_no) {
            cout << "| " << setw(NO_LEN) << std::right << setfill(' ') << "  No ";
        }
        var coli = 1;
        for (var i = 0; i < select->size(); ++i) {
            val as = select->at(i).as;
            cout << " | " << setw(cw.at(i)) << std::right << setfill(' ') <<
                 (as.empty() ? COL_PREFIX + to_string(coli) : as);
            coli++;
        }
        cout << " |" << endl;

        // dashes line
        val size = select->size();
        if (options->line_no) {
            cout << "|-" << setw(NO_LEN) << std::right << setfill('-') << "";
        }
        for (var i = 0; i < size; i++) {
            cout << "-+-" << setw(cw.at(i)) << std::right << setfill('-') << "";
        }
        cout << "-|" << endl;
    }

    var line_no = 1;
    for (val &row: *data) {
        if (options->line_no) {
            cout << "| " << setw(NO_LEN) << std::left << setfill(' ') << line_no++;
        }

        for (var i = 0; i < row->size(); ++i) {
            cout << " | " << setw(cw.at(i)) << std::right << setfill(' ') << row->at(i)->to_string();
        }
        cout << " |" << endl;
    }

    cout << endl << OK_MSG << flush;
}

ProgramOptions *options;
Map<String, Pair<Schema *, Result *>> *db;
