//
// Created by tika on 24-9-26.
//

#ifndef BASH_SQL_DATE_FUNCS_H
#define BASH_SQL_DATE_FUNCS_H

enum TimeUnit {
    I_NONE = 0,
    I_SECOND = 1,
    I_MINUTE = 60,
    I_HOUR = 60 * 60,
    I_DAY = 24 * 60 * 60,
    I_WEEK = 7 * 24 * 60 * 60,
};

using Time = std::tm;

#endif //BASH_SQL_DATE_FUNCS_H
