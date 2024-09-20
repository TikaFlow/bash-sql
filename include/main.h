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

ProgramOptions *parse_cmd_options(int argc, char *argv[]);

void handle_curd();

void interactive();

void prepare_data(const String &table);

void print_data(Result *data, Vector<SelectNode> *select);

#endif //BASH_SQL_MAIN_H
