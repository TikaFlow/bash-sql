//
// Created by tika on 24-5-7.
//

#ifndef BASH_SQL_GLOBAL_H
#define BASH_SQL_GLOBAL_H

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <iomanip>

// constants
#define NONE "<null>"

// version and copyright
#define VERSION "1.0.1"
#define AUTHOR "Tika Flow"

// keyword
#define var auto
#define val const auto
#define null nullptr

// type
using String = std::string;
template<typename T>
using Vector = std::vector<T>;
template<typename T>
using Stack = std::stack<T>;
template<typename K, typename V>
using Map = std::map<K, V>;
template<typename T1, typename T2>
using Pair = std::pair<T1, T2>;
using Regex = std::regex;

// function
using std::stoi;
using std::stol;
using std::stof;
using std::stod;
using std::to_string;
using std::for_each;
using std::all_of;
using std::sort;
using std::getline;
using std::endl;
using std::setw;
using std::setfill;

// object
using std::cin;
using std::cout;
using std::cerr;
val npos = String::npos;

#endif //BASH_SQL_GLOBAL_H
