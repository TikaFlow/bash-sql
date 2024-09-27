//
// Created by tika on 24-9-17.
//

#include "funcs.h"
#include "date_funcs.h"

static int GMT_OFFSET = 0;
static TimeUnit unites[] = {I_NONE, I_SECOND, I_MINUTE, I_HOUR, I_DAY, I_WEEK,};

/**
 * Convert timestamp to string
 * @param timestamp timestamp
 * @param fmt output string format, default: %F %T
 * @return datetime string
 */
static String from_timestamp(time_t timestamp, const String &fmt = "%F %T") {
    val tm = gmtime(&timestamp);
    if (!tm) { // failed to convert
        return {""};
    }

    char buf[64] = {0};
    strftime(buf, sizeof(buf), fmt.c_str(), tm);
    return {buf};
}

/**
 * Convert string to time
 * @param str datetime string
 * @param fmt input string format, default: %F %T
 * @return time
 */
static Time to_time(const String &str, const String &fmt = "%F %T") {
    var tm = Time();
    strptime(str.c_str(), fmt.c_str(), &tm);
    mktime(&tm);
    return tm;
}

/**
 * Convert string to timestamp
 * @param str datetime string
 * @param fmt input string format, default: %F %T
 * @return timestamp
 */
static time_t to_timestamp(const String &str, const String &fmt = "%F %T") {
    var tm = to_time(str, fmt);
    return mktime(&tm) + GMT_OFFSET;
}

/**
 * Convert interval to seconds
 * @param interval interval string, such as '1 02:03:04'
 * @return interval in seconds
 */
static int parseInterval(const String &interval) {
    if (interval.empty()) {
        return 0;
    }

    var str = interval;
    if (str.find(' ') == npos) {
        str = "0 " + str;
    }
    val day = str.substr(0, str.find(' '));
    val time = str.substr(str.find(' ') + 1);

    var res = 0, num = 0;
    char *endptr;
    if (day != "0") {
        num = (int) strtol(day.c_str(), &endptr, 10);
        if (endptr == day.c_str() || *endptr != '\0') {
            return 0;
        }

        res += num * I_DAY;
    }
    val h = time.substr(0, 2);
    val m = time.substr(3, 2);
    val s = time.substr(6, 2);
    num = (int) strtol(h.c_str(), &endptr, 10);
    if (endptr == h.c_str() || *endptr != '\0' || num < 0 || num >= 24) {
        return 0;
    }
    res += num * I_HOUR;
    num = (int) strtol(m.c_str(), &endptr, 10);
    if (endptr == m.c_str() || *endptr != '\0' || num < 0 || num >= 60) {
        return 0;
    }
    res += num * I_MINUTE;
    num = (int) strtol(s.c_str(), &endptr, 10);
    if (endptr == s.c_str() || *endptr != '\0' || num < 0 || num >= 60) {
        return 0;
    }
    res += num * I_SECOND;

    return res;
}

/**
 * aux function for date_add and date_sub
 * @param row params
 * @param func_name who calls this function
 * @param op operate function
 * @return result cell
 */
static Cell *date_add_sub_aux(Row *row, const String &func_name, long(*op)(long, long)) {
    check_arg_nums(row, func_name, 2, 3);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val bf_str = row->at(0)->text;
    var interval = 0, unit = 1;
    var af_str = String();
    if (row->at(1)->type == D_INTEGER) {
        interval = (int) row->at(1)->number;
    }
    if (row->size() == 3 && row->at(2)->type == D_INTEGER) {
        unit = (int) row->at(2)->number;
    }

    val bf_tt = to_timestamp(bf_str);
    if (unit < 1 || unit > 5) {
        return new Cell(bf_str);
    }
    val dur = interval * unites[unit];

    af_str = from_timestamp(op(bf_tt, dur), "%F");
    return new Cell(af_str);
}

/**
 * aux function for time add and sub
 * @param row params
 * @param func_name who calls this function
 * @param op operate function
 * @return result cell
 */
static Cell *time_add_sub_aux(Row *row, const String &func_name, long(*op)(long, long)) {
    check_arg_nums(row, func_name, 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val bf_str = row->at(0)->text;
    var interval = row->at(1)->text;
    var af_str = String();

    val bf_tt = to_timestamp(bf_str);
    val dur = parseInterval(interval);
    if (bf_tt < 0) {
        return new Cell(af_str);
    }

    af_str = from_timestamp(op(bf_tt, dur));
    return new Cell(af_str);
}

/**
 * aux function for who needs attribute of time
 * @param row params
 * @param func_name who calls this function
 * @param fmt format string
 * @param op operate function
 * @return result cell
 */
static Cell *tm_attr_aux(Row *row, const String &func_name, int(*op)(const Time &), const String &fmt = "%F %T") {
    check_arg_nums(row, func_name, 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->text;
    val tm = to_time(str, fmt);
    return new Cell(D_INTEGER, op(tm));
}

static Cell *tk_date_add(Row *row);

static Cell *tk_adddate(Row *row) {
    check_arg_nums(row, "adddate", 2);
    return tk_date_add(row);
}

static Cell *tk_addtime(Row *row) {
    return time_add_sub_aux(row, "addtime", [](long a, long b) { return a + b; });
}

static Cell *tk_curdate(Row *row) {
    check_arg_nums(row, "curdate", 0);
    val now = time(null);
    return new Cell(from_timestamp(now, "%F"));
}

static Cell *tk_current_date(Row *row) {
    check_arg_nums(row, "current_date", 0);
    return tk_curdate(row);
}

static Cell *tk_curtime(Row *row);

static Cell *tk_current_time(Row *row) {
    check_arg_nums(row, "current_time", 0);
    return tk_curtime(row);
}

static Cell *tk_now(Row *row);

static Cell *tk_current_timestamp(Row *row) {
    check_arg_nums(row, "current_timestamp", 0);
    return tk_now(row);
}

static Cell *tk_curtime(Row *row) {
    check_arg_nums(row, "curtime", 0);
    val now = time(null);
    return new Cell(from_timestamp(now, "%T"));
}

static Cell *tk_date(Row *row) {
    check_arg_nums(row, "date", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->text;
    return new Cell(str.substr(0, 10));
}

static Cell *tk_datediff(Row *row) {
    check_arg_nums(row, "datediff", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val a_str = row->at(0)->text;
    val b_str = row->at(1)->text;

    val diff = to_timestamp(a_str, "%F") - to_timestamp(b_str, "%F");
    return new Cell(D_INTEGER, (int) (diff / I_DAY));
}

static Cell *tk_date_add(Row *row) {
    return date_add_sub_aux(row, "date_add", [](long a, long b) { return a + b; });
}

static Cell *tk_date_sub(Row *row) {
    return date_add_sub_aux(row, "date_sub", [](long a, long b) { return a - b; });
}

static Cell *tk_dayofmonth(Row *row);

static Cell *tk_day(Row *row) {
    check_arg_nums(row, "day", 1);
    return tk_dayofmonth(row);
}

static Cell *tk_dayname(Row *row) {
    check_arg_nums(row, "dayname", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val date = to_timestamp(row->at(0)->text);
    if (date < 0) {
        return new Cell();
    }

    val ori_lc = std::locale::global(std::locale("")); // set locale to default (LC_TIME)
    var day_name = from_timestamp(date, "%A");
    std::locale::global(ori_lc);

    return new Cell(day_name);
}

static Cell *tk_dayofmonth(Row *row) {
    val res = tm_attr_aux(row, "dayofmonth", [](const Time &tm) { return tm.tm_mday; });
    if (to_timestamp(row->at(0)->text) < 0) {
        res->number = 0;
    }
    return res;
}

static Cell *tk_dayofweek(Row *row) {
    return tm_attr_aux(row, "dayofweek", [](const Time &tm) { return tm.tm_wday + 1; });
}

static Cell *tk_dayofyear(Row *row) {
    return tm_attr_aux(row, "dayofyear", [](const Time &tm) { return tm.tm_yday + 1; });
}

static Cell *tk_from_unixtime(Row *row) {
    check_arg_nums(row, "from_unixtime", 1, 2);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    var ts = 0;
    if (row->at(0)->type == D_INTEGER) {
        ts = (int) row->at(0)->number;
    }
    var fmt = String("%F %T");
    if (row->size() == 2 && row->at(1)->type == D_STRING) {
        fmt = row->at(1)->text;
    }
    return new Cell(from_timestamp(ts + GMT_OFFSET, fmt));
}

static Cell *tk_hour(Row *row) {
    return tm_attr_aux(row, "hour", [](const Time &tm) { return tm.tm_hour; }, "%T");
}

static Cell *tk_last_day(Row *row) {
    val res = tm_attr_aux(row, "last_day", [](const Time &tm) { return tm.tm_mon + 1; });
    val mon = (int) res->number;

    var tm = to_time(row->at(0)->text);
    if (mktime(&tm) < 0) {
        return new Cell();
    }
    switch (mon) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            res->number = 31;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            res->number = 30;
            break;
        case 2:
            if (is_leap_year(tm.tm_year + 1900)) {
                res->number = 29;
            } else {
                res->number = 28;
            }
            break;
        default:
            break; // make compiler happy
    }
    return res;
}

static Cell *tk_localtime(Row *row) {
    check_arg_nums(row, "localtime", 0);
    return tk_now(row);
}

static Cell *tk_localtimestamp(Row *row) {
    check_arg_nums(row, "localtimestamp", 0);
    return tk_now(row);
}

static Cell *tk_makedate(Row *row) {
    check_arg_nums(row, "makedate", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    var year = 0;
    if (row->at(0)->type == D_INTEGER) {
        year = (int) row->at(0)->number;
    }
    var dayofyear = 0;
    if (row->at(1)->type == D_INTEGER) {
        dayofyear = (int) row->at(1)->number;
    }

    val timestamp = to_timestamp(to_string(year) + "-01-01");
    if (dayofyear < 1 || timestamp < 0) {
        return new Cell();
    }
    return new Cell(from_timestamp(timestamp + (dayofyear - 1) * I_DAY, "%F"));
}

static Cell *tk_maketime(Row *row) {
    check_arg_nums(row, "maketime", 3);

    if (check_null(row->at(0)) || check_null(row->at(1)) || check_null(row->at(2))) {
        return new Cell();
    }

    var hour = 0;
    if (row->at(0)->type == D_INTEGER) {
        hour = (int) row->at(0)->number;
    }
    var minute = 0;
    if (row->at(1)->type == D_INTEGER) {
        minute = (int) row->at(1)->number;
    }
    var socond = 0;
    if (row->at(2)->type == D_INTEGER) {
        socond = (int) row->at(2)->number;
    }

    val timestamp = to_timestamp("1970-01-01 " + to_string(hour) + ":" + to_string(minute) + ":" + to_string(socond));
    if (timestamp < 0) {
        return new Cell();
    }
    return new Cell(from_timestamp(timestamp, "%T"));
}

static Cell *tk_minute(Row *row) {
    return tm_attr_aux(row, "minute", [](const Time &tm) { return tm.tm_min; });
}

static Cell *tk_month(Row *row) {
    val res = tm_attr_aux(row, "month", [](const Time &tm) { return tm.tm_mon + 1; });
    if (to_timestamp(row->at(0)->text) < 0) {
        res->number = 0;
    }
    return res;
}

static Cell *tk_monthname(Row *row) {
    check_arg_nums(row, "monthname", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val date = to_timestamp(row->at(0)->text);
    if (date < 0) {
        return new Cell();
    }

    val ori_lc = std::locale::global(std::locale(""));
    val month_name = from_timestamp(date, "%B");
    std::locale::global(ori_lc);

    return new Cell(month_name);
}

static Cell *tk_now(Row *row) {
    check_arg_nums(row, "now", 0);
    val now = time(null);
    return new Cell(from_timestamp(now));
}

static Cell *tk_quarter(Row *row) {
    return tm_attr_aux(row, "quarter", [](const Time &tm) { return tm.tm_mon / 3 + 1; });
}

static Cell *tk_second(Row *row) {
    return tm_attr_aux(row, "second", [](const Time &tm) { return tm.tm_sec; }, "%T");
}

static Cell *tk_sec_to_time(Row *row) {
    check_arg_nums(row, "sec_to_time", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    var secs = 0;
    if (row->at(0)->type == D_INTEGER) {
        secs = (int) row->at(0)->number;
    }

    return new Cell(from_timestamp(secs, "%T"));
}

static Cell *tk_str_to_date(Row *row) {
    check_arg_nums(row, "str_to_date", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val str = row->at(0)->text;
    var fmt = String("%F");
    if (row->at(1)->type == D_STRING) {
        fmt = row->at(1)->text;
    }

    val tm = to_timestamp(str, fmt);
    if (tm < 0) {
        return new Cell();
    }
    if (tm < I_DAY) {
        return new Cell(from_timestamp(tm, "%T"));
    }
    return new Cell(from_timestamp(tm, "%F %T"));
}

static Cell *tk_subdate(Row *row) {
    check_arg_nums(row, "subdate", 2);
    return tk_date_sub(row);
}

static Cell *tk_subtime(Row *row) {
    return time_add_sub_aux(row, "subtime", [](long a, long b) { return a - b; });
}

static Cell *tk_sysdate(Row *row) {
    check_arg_nums(row, "sysdate", 0);
    return tk_now(row);
}

static Cell *tk_time(Row *row) {
    check_arg_nums(row, "time", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->text;
    val tm = to_timestamp(str);
    if (tm < 0) {
        return new Cell();
    }
    return new Cell(from_timestamp(tm, "%T"));
}

static Cell *tk_timediff(Row *row) {
    check_arg_nums(row, "timediff", 2);

    if (check_null(row->at(0)) || check_null(row->at(1))) {
        return new Cell();
    }

    val a = to_timestamp(row->at(0)->text);
    val b = to_timestamp(row->at(1)->text);
    var diff = a - b;

    val neg = diff < 0;
    diff = abs((int) diff);

    var days = 0;
    if (diff >= I_DAY) {
        days = (int) (diff / I_DAY);
        diff -= days * I_DAY;
    }

    var str = from_timestamp(diff, "%T");
    val hour = stoi(str.substr(0, 2));
    str.replace(0, 2, to_string(days * 24 + hour));

    if (neg) {
        str = "-" + str;
    }

    return new Cell(str);
}

static Cell *tk_time_to_sec(Row *row) {
    check_arg_nums(row, "time_to_sec", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = "1970-01-01 " + row->at(0)->text;
    return new Cell(D_INTEGER, (int) to_timestamp(str));
}

static Cell *tk_unix_timestamp(Row *row) {
    check_arg_nums(row, "unix_timestamp", 0, 1);

    if (row->empty()) {
        return new Cell(D_INTEGER, (int) time(null));
    }

    if (check_null(row->at(0))) {
        return new Cell();
    }

    return new Cell(D_INTEGER, (int) to_timestamp(row->at(0)->text) - GMT_OFFSET);
}

static Cell *tk_week(Row *row) {
    /*
     * <str>, <first_day>, <mode>
     * mode: 0 - week in range 0-53, and week0 has <first_day>
     *       1 - week in range 0-53, and week0 has 4 or more days
     *       2 - week in range 1-53, and week1 has <first_day>
     *       3 - week in range 1-53, and week1 has 4 or more days (default)
     */
    check_arg_nums(row, "week", 1, 3);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val str = row->at(0)->text;
    var first_day = 1;
    var mode = 3;
    if (row->size() > 1 && row->at(1)->type == D_INTEGER) {
        first_day = (int) row->at(1)->number;
        if (first_day < 0 || first_day > 6) {
            first_day = 1;
        }
    }
    if (row->size() > 2 && row->at(2)->type == D_INTEGER) {
        mode = (int) row->at(2)->number;
        if (mode < 0 || mode > 3) {
            mode = 3;
        }
    }

    var tm = to_time(str);
    tm.tm_isdst = 0;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    val the_date = mktime(&tm) + GMT_OFFSET;

    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_year += 1;
    val year_last = mktime(&tm) + GMT_OFFSET - I_DAY;

    var dangerous_days = (tm.tm_wday + 6) % 7 - first_day;
    if (dangerous_days < 0) {
        dangerous_days += 7;
    }
    if (!(dangerous_days < 3 && mode % 2)) {
        dangerous_days = -1;
    }
    if (year_last - the_date <= dangerous_days * I_DAY) {
        return new Cell(D_INTEGER, (int) (mode / 2));
    }

    tm.tm_year -= 1;
    var year_first = mktime(&tm) + GMT_OFFSET;

    var days_to_day0 = (int) (first_day - tm.tm_wday + 7) % 7;
    if (days_to_day0 > 3 && mode % 2) {
        days_to_day0 -= 7;
    }
    var day0 = year_first + days_to_day0 * I_DAY;

    var tt_diff = the_date - day0;
    if (tt_diff < 0) {
        tm.tm_year -= 1;
        year_first = mktime(&tm) + GMT_OFFSET;

        days_to_day0 = (int) (first_day - tm.tm_wday + 7) % 7;
        if (days_to_day0 > 3 && mode % 2) {
            days_to_day0 -= 7;
        }
        day0 = year_first + days_to_day0 * I_DAY;
        tt_diff = the_date - day0;
    }
    val weeks = (int) (tt_diff) / I_WEEK + mode / 2;

    return new Cell(D_INTEGER, weeks);
}

static Cell *tk_weekday(Row *row) {
    return tm_attr_aux(row, "weekday", [](const Time &tm) { return tm.tm_wday; });
}

static Cell *tk_weekofyear(Row *row) {
    check_arg_nums(row, "weekofyear", 1);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    var first_day = Cell(D_INTEGER, 1);
    var mode = Cell(D_INTEGER, 3);
    val new_row = new Row();
    new_row->push_back(row->at(0));
    new_row->push_back(&first_day);
    new_row->push_back(&mode);

    return tk_week(new_row);
}

static Cell *tk_year(Row *row) {
    return tm_attr_aux(row, "year", [](const Time &tm) { return tm.tm_year + 1900; });
}

static Cell *tk_yearweek(Row *row) {
    check_arg_nums(row, "yearweek", 1, 3);

    if (check_null(row->at(0))) {
        return new Cell();
    }

    val res = tk_week(row);
    val tm = to_time(row->at(0)->text);
    var year = tm.tm_year;
    if ((int) res->number > 50 && tm.tm_yday < 14) {
        year--;
    } else if ((int) res->number == 1 && tm.tm_yday > 350) {
        year++;
    }
    res->text = to_string(year + 1900);

    res->text += res->number < 10 ? "0" : "";
    res->text += to_string((int) res->number);

    res->number = 0;
    res->type = D_STRING;

    return res;
}

void init_date_funcs() {
    val now = time(null);
    val gmt = localtime(&now);
    GMT_OFFSET = (int) gmt->tm_gmtoff;

    ADD_NORMAL(adddate, D_STRING);
    ADD_NORMAL(addtime, D_STRING);
    ADD_NORMAL(curdate, D_STRING);
    ADD_NORMAL(current_date, D_STRING);
    ADD_NORMAL(current_time, D_STRING);
    ADD_NORMAL(current_timestamp, D_STRING);
    ADD_NORMAL(curtime, D_STRING);
    ADD_NORMAL(date, D_STRING);
    ADD_NORMAL(datediff, D_INTEGER);
    ADD_NORMAL(date_add, D_STRING);
    ADD_NORMAL(date_sub, D_STRING);
    ADD_NORMAL(day, D_INTEGER);
    ADD_NORMAL(dayname, D_STRING);
    ADD_NORMAL(dayofmonth, D_INTEGER);
    ADD_NORMAL(dayofweek, D_INTEGER);
    ADD_NORMAL(dayofyear, D_INTEGER);
    ADD_NORMAL(from_unixtime, D_STRING);
    ADD_NORMAL(hour, D_INTEGER);
    ADD_NORMAL(last_day, D_STRING);
    ADD_NORMAL(localtime, D_STRING);
    ADD_NORMAL(localtimestamp, D_STRING);
    ADD_NORMAL(makedate, D_STRING);
    ADD_NORMAL(maketime, D_STRING);
    ADD_NORMAL(minute, D_INTEGER);
    ADD_NORMAL(month, D_INTEGER);
    ADD_NORMAL(monthname, D_STRING);
    ADD_NORMAL(now, D_STRING);
    ADD_NORMAL(quarter, D_INTEGER);
    ADD_NORMAL(second, D_INTEGER);
    ADD_NORMAL(sec_to_time, D_STRING);
    ADD_NORMAL(str_to_date, D_STRING);
    ADD_NORMAL(subdate, D_STRING);
    ADD_NORMAL(subtime, D_STRING);
    ADD_NORMAL(sysdate, D_STRING);
    ADD_NORMAL(time, D_STRING);
    ADD_NORMAL(timediff, D_STRING);
    ADD_NORMAL(time_to_sec, D_INTEGER);
    ADD_NORMAL(unix_timestamp, D_INTEGER);
    ADD_NORMAL(week, D_INTEGER);
    ADD_NORMAL(weekday, D_INTEGER);
    ADD_NORMAL(weekofyear, D_INTEGER);
    ADD_NORMAL(year, D_INTEGER);
    ADD_NORMAL(yearweek, D_INTEGER);
}
