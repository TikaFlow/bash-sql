//
// Created by tika on 24-9-21.
//

#include "defs.h"
#include "util.h"

String Token::to_string() const {
    std::ostringstream out;
    switch (this->type) {
        case T_INTEGER:
            return ::to_string(this->integer);
        case T_REAL:
            out << std::fixed << std::setprecision(PRECISION) << this->real;
            return cut_tail(out.str());
        default:
            return this->text;
    }
}

bool ASTNode::tableless() const {
    if (this->atype == A_COLUMN) {
        return false;
    }

    if (this->left) {
        if (!left->tableless()) {
            return false;
        }
    }

    if (this->right) {
        if (!this->right->tableless()) {
            return false;
        }
    }

    return true;
}

bool SelectStatement::tableless_with() const {
    if (!this->with) {
        return true;
    }

    return all_of(this->with->begin(), this->with->end(), [](WithNode &node) { return node.stmt->tableless(); });
}

bool SelectStatement::tableless_select() const {
    if (!this->select) {
        return true;
    }

    return all_of(this->select->begin(), this->select->end(), [](SelectNode &node) { return node.col->tableless(); });
}

bool SelectStatement::tableless() const {
    return tableless_with() && tableless_select();
}

String Cell::to_string() const {
    std::ostringstream out;
    switch (this->type) {
        case D_BOOL:
            return this->number == 0 ? "false" : "true";
        case D_INTEGER:
        case D_REAL:
            if (this->text == NONE) {
                return NONE;
            }
            out << std::fixed << std::setprecision(PRECISION) << this->number;
            return cut_tail(out.str());
        case D_STRING:
            return this->text;
        default:
            return ""; // make compiler happy
    }
}
