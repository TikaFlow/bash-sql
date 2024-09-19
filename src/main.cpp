//
// Created by tika on 4/21/24.
//

#include "main.h"

#define ERROR_MSG "1 ERROR."
#define OK_MSG "OK."

bool INTERACTIVE_MODE = false;

int main(int argc, char *argv[]) {
    signal(SIGINT, [](int) {
        if (INTERACTIVE_MODE) {
            cout << endl << "Please type 'exit' to exit." << endl << " (sql) > " << flush;
        } else {
            exit(130);
        }
    });

    // initialize functions mapping
    init_funcs();

    // parse command line options
    val options = parse_cmd_options(argc, argv);

    if (options->interactive) {
        return read_and_exec(options);
    }

    return handle_curd(options);
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
    val options = new ProgramOptions{false, false, false, "", "", '\0', 0, ""};

    var help = false;
    var version = false;
    var data = false;
    var file = false;

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
            options->data += line + "\n";
        }
        if (!options->data.empty()) {
            options->data.pop_back();
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
                options->title = true;
                break;
            case 'l':
                options->line_no = true;
                break;
            case 'i':
                options->interactive = true;
                break;
            case 'f':
                // check file exists
                if (access(optarg, F_OK)) {
                    arg_error("File not found: " + String(optarg));
                }
                file = true;
                options->file = optarg;
                break;
            case 'd':
                options->delimiter = optarg[0];
                break;
            case 'c':
                options->columns = stoi(optarg);
                if (options->columns < 0) {
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

    // query string
    for (var i = optind; i < argc; ++i) {
        options->query += argv[i];
        if (i < argc - 1) {
            options->query += " ";
        }
    }

    // check mutually exclusive options
    if (help || version) {
        if (options->title || data || file || options->delimiter || options->columns || !options->query.empty()) {
            if (help) {
                arg_error("Help option cannot be used with other options.");
            }
            if (version) {
                arg_error("Version option cannot be used with other options.");
            }
        }
    } else {
        if (data && file) {
            arg_error("Only file or standard input should be specified.");
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

    // if column number is not specified, use default value
    if (!options->columns) {
        options->columns = 32;
    }

    // get data string
    if (file) {
        options->data = read_file_to_string(options->file);
    }

    return options;
}

/**
 * Read source file and return data string
 * @param line source command line
 * @param columns column number to save in
 * @param delimiter delimiter to save in
 * @return data string
 */
static String source_file(const String &line, int &columns, char &delimiter) {
    val rest = trim(line.substr(6, line.length() - 6));
    val args = split_string_by_spaces(rest);

    if (args->empty()) {
        show_warn("Invalid source command: " + line);
        cout << "Usage: source <file> [colum count [delimiter]]" << endl
             << endl << ERROR_MSG << flush;
        return {};
    }

    val data = read_file_to_string(args->at(0));
    if (args->size() >= 2) {
        columns = stoi(args->at(1));
    } else {
        columns = 32;
    }
    if (args->size() >= 3) {
        delimiter = args->at(2)[0];
    } else {
        delimiter = '\0';
    }

    return data;
}

/**
 * Handle CURD operations
 * @param options CURD options
 * @return 0 if success
 */
int handle_curd(ProgramOptions *options) {
    val queries = parse_sql(options->query, options->columns);
    for (val &query: *queries) {
        val result = apply(query, options->data, options->columns, options->delimiter);
        print_data(result, query->select, options->title, options->line_no);
    }

    return 0;
}

/**
 * Read and execute SQL commands in interactive mode
 * @param options CURD options
 * @return 0 if success
 */
int read_and_exec(ProgramOptions *options) {
    INTERACTIVE_MODE = true;
    // var tables = Vector<String>();

    while (true) {
        cout << endl << " (sql) > " << flush;

        var line = String();
        std::getline(std::cin, line);
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        val first = split_string_by_spaces(line)->at(0);
        if (first == "exit" || first == "exit()" || first == "exit;" || first == "exit();") {
            cout << "Bye!" << endl;
            break;
        } else if (first == "source") {
            options->data = source_file(line, options->columns, options->delimiter);
            continue;
        } else if (first == "help") {
            show_help();
            continue;
        }

        options->query = line;
        if (fork()) {
            int status;
            waitpid(-1, &status, 0);

            if (status) {
                cout << endl << ERROR_MSG;
            } else {
                cout << endl << OK_MSG;
            }
        } else {
            return handle_curd(options);
        }
    }

    return 0;
}

/**
 * Print data
 * @param data data
 * @param select select nodes
 * @param print_title print title or not
 * @param print_line_no print line number or not
 */
void print_data(Result *data, Vector<SelectNode> *select, bool print_title, bool print_line_no) {
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

    if (print_title) {
        // title
        if (print_line_no) {
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
        if (print_line_no) {
            cout << "|-" << setw(NO_LEN) << std::right << setfill('-') << "";
        }
        for (var i = 0; i < size; i++) {
            cout << "-+-" << setw(cw.at(i)) << std::right << setfill('-') << "";
        }
        cout << "-|" << endl;
    }

    var line_no = 1;
    for (val &row: *data) {
        if (print_line_no) {
            cout << "| " << setw(NO_LEN) << std::left << setfill(' ') << line_no++;
        }

        for (var i = 0; i < row->size(); ++i) {
            cout << " | " << setw(cw.at(i)) << std::right << setfill(' ') << row->at(i)->to_string();
        }
        cout << " |" << endl;
    }
}
