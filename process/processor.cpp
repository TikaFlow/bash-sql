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
            return apply_create(stmt.name, stmt.stmt_select);
        case S_INSERT:
            // return apply_insert(stmt.stmt_insert);
        case S_UPDATE:
            return apply_update(stmt.stmt_update);
        case S_SELECT:
            return apply_read(stmt.stmt_select);
        case S_DROP:
            return apply_drop(stmt.name);
        case S_DELETE:
            return apply_delete(stmt.stmt_delete);
        case S_DESCRIBE:
            return apply_describe(stmt.name);
        case S_SHOW:
            return apply_show();
        case S_BANG:
            return apply_bang(stmt.number);
        case S_HISTORY:
            return apply_history();
        default:
            show_error("Unsupported statement type");
            return null; // make compiler happy
    }
}
