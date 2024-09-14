//
// Created by tika on 4/21/24.
//

#include "util.h"

namespace util {
    String ltrim(const String &str) {
        val start = str.find_first_not_of(" \t\n\r\f\v");
        return start == npos ? "" : str.substr(start);
    }

    String rtrim(const String &str) {
        val end = str.find_last_not_of(" \t\n\r\f\v");
        return end == npos ? "" : str.substr(0, end + 1);
    }

    String trim(const String &str) {
        return rtrim(ltrim(str));
    }

    String to_lower(const String &str) {
        String res;
        std::transform(str.begin(), str.end(), std::back_inserter(res), ::tolower);
        return res;
    }

    Vector<String> *split_string(const String &query, char delimiter) {
        val res = new Vector<String>();
        size_t pos = 0;
        var found = query.find(delimiter);
        while (found != npos) {
            res->push_back(query.substr(pos, found - pos));
            pos = found + 1;
            found = query.find(delimiter, pos);
        }
        res->push_back(query.substr(pos));
        return res;
    }

    Vector<String> *split_string_by_spaces(const String &input) {
        const Regex re("\\s+");

        std::sregex_token_iterator it(input.begin(), input.end(), re, -1);
        val res = new Vector<String>();

        for (const std::sregex_token_iterator end; it != end; ++it) {
            res->push_back(it->str());
        }

        res->erase(std::remove_if(res->begin(), res->end(), [](const String &s) {
            return s.empty();
        }), res->end());

        return res;
    }

    void show_error(const String &msg) {
        cerr << "ERROR: " << msg << endl;
        exit(1);
    }

    void show_warn(const String &msg) {
        cout << "WARNING: " << msg << endl;
    }

    String &replace_all(String &str, const String &src, const String &dst) {
        String::size_type pos;
        while (true) {
            if ((pos = str.find(src)) != npos) {
                str.replace(pos, src.length(), dst);
            } else {
                break;
            }
        }
        return str;
    }

    String read_file_to_string(const String &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            show_error("Unable to open file: " + filename);
        }

        String content{(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
        return trim(content);
    }

    bool is_integer(const String &str) {
        std::istringstream iss(str);
        long value;
        iss >> value;
        return iss.eof() && !iss.fail();
    }

    bool is_double(const String &str) {
        std::istringstream iss(str);
        double value;
        iss >> value;
        return iss.eof() && !iss.fail();
    }

    void dump_ast(ASTNode *node, const String &indent = "") {
        cout << "implement it later" << endl;
    }

    int compare_number(double a, double b) {
        if (a < b) {
            return -1;
        } else if (a > b) {
            return 1;
        } else {
            return 0;
        }
    }

    String cut_tail(const String &str) {
        val pos = str.find_last_not_of('0');

        if (pos == npos) {
            return str;
        }

        if (str[pos] == '.') {
            return str.substr(0, pos);
        }

        return str.substr(0, pos + 1);
    }
}