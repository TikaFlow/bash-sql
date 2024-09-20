//
// Created by tika on 24-9-20.
//

#include "process.h"

/**
 * Apply one sql to the data
 * @param query the sql AST
 * @param data the origin data to be processed, not included the title
 * @param col_count the column count of the data
 * @param d the delimiter of the data, default is "\\s+"
 * @return the processed data
 */
Result *process(Statement stmt) {
    switch (stmt.type) {
        case S_SELECT:
            return apply_read(stmt.stmt_r, options->data, options->columns, options->delimiter);
        default:
            show_error("Unsupported statement type");
    }

    return null; // make compiler happy
}
