//
// Created by tika on 4/22/24.
//

#ifndef BASH_SQL_MAIN_H
#define BASH_SQL_MAIN_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <getopt.h>
#include <sys/wait.h>
#include <csignal>
#include "parser.h"
#include "process.h"

#define NO_LEN 5
#define COL_INIT_LEN 8
#define COL_PREFIX "__col_"

// declare struct
struct ProgramOptions {
    bool title;
    bool line_no;
    bool interactive;
    String data;
    String file;
    char delimiter;
    int columns;
    String query;
};

ProgramOptions *parse_cmd_options(int argc, char *argv[]);

int handle_curd(ProgramOptions *options);

int read_and_exec(ProgramOptions *options);

void print_data(Result *data, Vector<SelectNode> *select, bool print_title, bool print_line_no);

#endif //BASH_SQL_MAIN_H
