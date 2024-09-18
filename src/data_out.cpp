//
// Created by tika on 24-5-11.
//

#include "data_out.h"

void print_data(Result *data, Vector<SelectNode> *select, bool print_title, bool print_line_no) {
    if (print_title) {
        // title
        if (print_line_no) {
            cout << "| " << setw(6) << std::right << setfill(' ') << "No ";
        }
        var coli = 1;
        for (val &col: *select) {
            cout << " | " << setw(16) << std::right << setfill(' ') <<
                 (col.as.empty() ? "_col_" + to_string(coli) : col.as);
            coli++;
        }
        cout << " |" << endl;

        // dashes line
        val size = select->size();
        if (print_line_no) {
            cout << "|" << setw(8) << std::right << setfill('-') << "";
        }
        for (var i = 0; i < size; i++) {
            cout << "+" << setw(18) << std::right << setfill('-') << "";
        }
        cout << "|" << endl;
    }

    var line_no = 1;
    for (val &row: *data) {
        if (print_line_no) {
            cout << "| " << setw(6) << std::right << setfill(' ') << line_no++;
        }

        for (auto cell: *row) {
            cout << " | " << setw(16) << std::right << setfill(' ') << cell->to_string();
        }
        cout << " |" << endl;
    }
}