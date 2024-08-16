//
// Created by tika on 4/21/24.
//

#include "main.h"

int main(int argc, char *argv[]) {
    // parse command line options
    val options = parse_cmd_options(argc, argv);

    // get parsed sql ast
    val sql = parse_sql(options.query);

    // process data
    var data = process_data(options);

    // print data
    print_data(data, options);

    return 0;
}
