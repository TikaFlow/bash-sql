//
// Created by tika on 4/21/24.
//

#ifndef BASH_SQL_GETOPT_UTIL_H
#define BASH_SQL_GETOPT_UTIL_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <getopt.h>
#include "util.h"

// declare struct
struct ProgramOptions {
    bool title;
    bool line_no;
    String data;
    String file;
    char delimiter;
    int columns;
    String query;
};

void show_version();
void arg_error(const String &error);
ProgramOptions parse_cmd_options(int argc, char* argv[]);


#endif //BASH_SQL_GETOPT_UTIL_H
