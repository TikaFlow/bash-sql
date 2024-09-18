//
// Created by tika on 4/21/24.
//

#include "main.h"

int main(int argc, char *argv[]) {
    // initialize functions mapping
    init_funcs();

    // parse command line options
    val options = parse_cmd_options(argc, argv);
    val columns = options.columns; // must be greater than 0

    // get parsed queries ast
    val queries = parse_sql(options.query, columns);

    for (auto query: *queries) {
        // process data
        val data = apply(query, options.data, columns, options.delimiter);
        // print data
        print_data(data, query->select, options.title, options.line_no);
    }

    return 0;
}
