//
// Created by tika on 24-9-20.
//

#include "process.h"

/**
 * Apply one sql to the data
 * @param stmt the sql statement
 * @return the processed data
 */
Result *process(const Statement &stmt) {
    switch (stmt.type) {
        case S_CREATE:
            return apply_create(stmt.name, stmt.stmt_r);
        case S_SELECT:
            return apply_read(stmt.stmt_r);
        case S_DELETE:
            return apply_delete(stmt.name);
        case S_DESCRIBE:
            return apply_describe(stmt.name);
        case S_SHOW:
            return apply_show();
        default:
            show_error("Unsupported statement type");
            return null; // make compiler happy
    }
}
