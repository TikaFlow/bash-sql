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

void import_data(const String &table);

void export_data(const String &table, const String &file, bool with_title, bool with_line_no, char deli);

void print_read(Result *data, Vector<SelectNode> *select, const Function<void()> &func = null);

void print_show(Result *data, const String &title, const Function<void()> &func = null);

#endif //BASH_SQL_MAIN_H
