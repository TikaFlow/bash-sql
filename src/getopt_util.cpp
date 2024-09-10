//
// Created by tika on 4/21/24.
//

#include "getopt_util.h"

void arg_error(const String &error) {
    show_error(error + "\nUse -h for help.");
}

void show_version() {
    cout << "Bash-sql V" << VERSION << " by " << AUTHOR << endl;
}

void show_help() {
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

ProgramOptions parse_cmd_options(int argc, char *argv[]) {
    ProgramOptions options = {false, false, "", "", '\0', 0, ""};

    var help = false;
    var version = false;
    var data = false;
    var file = false;

    static option long_options[] = {
            {"help",      no_argument,       null, 'h'},
            {"version",   no_argument,       null, 'v'},
            {"title",     no_argument,       null, 't'},
            {"line-no",   no_argument,       null, 'l'},
            {"file",      required_argument, null, 'f'},
            {"delimiter", required_argument, null, 'd'},
            {"columns",   required_argument, null, 'c'},
            {null, 0,                        null, 0}
    };

    // check if there is data from stdin
    if (isatty(STDIN_FILENO) == 0) {
        data = true;
        String line;
        while (getline(cin, line)) {
            options.data += line + "\n";
        }
        if (!options.data.empty()) {
            options.data.pop_back();
        }
    }

    int c;
    while ((c = getopt_long(argc, argv, "hvtld:f:c:", long_options, null)) != -1) {
        switch (c) {
            case 'h':
                help = true;
                break;
            case 'v':
                version = true;
                break;
            case 't':
                options.title = true;
                break;
            case 'l':
                options.line_no = true;
                break;
            case 'f':
                // check file exists
                if (access(optarg, F_OK)) {
                    arg_error("File not found: " + String(optarg));
                }
                file = true;
                options.file = optarg;
                break;
            case 'd':
                options.delimiter = optarg[0];
                break;
            case 'c':
                options.columns = stoi(optarg);
                if (options.columns < 0) {
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
    for (int i = optind; i < argc; ++i) {
        options.query += argv[i];
        if (i < argc - 1) {
            options.query += " ";
        }
    }

    // check mutually exclusive options
    if (help || version) {
        if (options.title || data || file || options.delimiter || options.columns || !options.query.empty()) {
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
    if (!options.columns) {
        options.columns = 32;
    }

    // get data string
    if (file) {
        options.data = read_file_to_string(options.file);
    }

    return options;
}