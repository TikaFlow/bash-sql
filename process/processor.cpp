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
    Result *res = null;

    switch (stmt.type) {
        case S_CREATE:
            res = apply_create(stmt.stmt_c);
            break;
        case S_SELECT:
            res = apply_read(stmt.stmt_r);
            break;
        default:
            show_error("Unsupported statement type");
    }

    return res; // make compiler happy
}
