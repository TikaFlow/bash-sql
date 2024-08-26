//
// Created by tika on 24-5-11.
//

#include "data_out.h"

void print_data(const Vector<Vector<String>> &data, const ProgramOptions &options) {
    // column widths
    Vector<int> col_ws;
    for (val &row: data) {
        for (var i = 0; i < row.size(); ++i) {
            if (col_ws.size() <= i) {
                col_ws.push_back(0);
            }
            col_ws[i] = std::max(col_ws[i], (int) (row[i].size()));
        }
    }

    // header
    if (options.title) {
        if (options.line_no) {
            cout << setw(4) << std::right << "No" << " | ";
        }
        for (var i = 0; i < data[0].size(); ++i) {
            cout << setw(col_ws[i]) << std::left << data[0][i];
            if (i < data[0].size() - 1) {
                cout << " | ";
            }
        }
        cout << endl;
        // dashed line
        if (options.line_no) {
            cout << setfill('-') << setw(6) << std::right << "+";
        }
        for (var i = 0; i < data[0].size(); ++i) {
            cout << setfill('-') << setw(col_ws[i] + 2) << std::left << "-";
            if (i < data[0].size() - 1) {
                cout << "+";
            }
        }
        cout << endl;
    }

    // data
    for (var row = 1, lineNum = 1; row < data.size(); ++row, ++lineNum) {
        if (options.line_no) {
            cout << setfill(' ') << setw(4) << std::right << lineNum << " | ";
        }
        for (var i = 0; i < data[row].size(); ++i) {
            cout << setfill(' ') << setw(col_ws[i]) << std::left << data[row][i];
            if (i < data[row].size() - 1) {
                cout << " | ";
            }
        }
        cout << endl;
    }
}