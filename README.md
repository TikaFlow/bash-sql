English | [简体中文](README_zh-CN.md)

# Bash-SQL

Using SQL-like languages to process lightweight data in bash.

❗ **DO NOT USE THIS PROGRAM IN PRODUCTION ENVIRONMENT**, because:

❕ all number types are treated as `double` in this program(may loss curacy), and

❕ not performance tested, only recommended for handling lightweight data.

# Installation

## #0. requirements

- cmake
- make
- g++

## #1. download the source code by `git clone` or any way you like

## #2. enter repo directory, create and enter the `build` directory

```bash
cd bash-sql
mkdir build && cd build
```

## #3. generate Makefile and specify the installation path

```bash
cmake -DCMAKE_INSTALL_PREFIX=/your/custom/prefix ..
```

## #4. compile and install the program

```bash
make && make install
```

# Usage

```bash
sql [OPTION] [QUERIES]
```

> Query statements must be enclosed in double quotes.

## CLI options

- `-h`, `--help`: Display this help and exit.
- `-v`, `--version`: Output version information and exit.
- `-t`, `--title`: Print table title.
- `-l`, `--line-no`: Print line number.
- `-i`, `--interactive`: Interactive mode.
- `-f`, `--file=FILE`: Read data from FILE.
- `-d`, `--delimiter=DELIMITER`: Use DELIMITER as field delimiter.
- `-c`, `--columns=COLUMNS`: Use COLUMNS as number of columns.

## Queries

Queries are SQL-like select statements:

```
[WITH] select {columns} [FROM] [WHERE] [GROUP BY] [ORDER BY] [LIMITS]
```

> Keywords and functions are case-insensitive.

The grammar is generally consistent with standard `SQL`, but:

- The input data is placed in a table called `std`.
- Subquery is not supported, please use `with` instead.
- Group by allows only field names or field indexes(in select clauses).

## Example

```bash
ps -aux | sql -tlc11 "select col1 as user, col2 as pid, col9 as start, col11 as command \
from std where col2 not like 'PID';"
```

## Supported functions

> 🟡 planning
>
> 🟢 implemented

### type conversion

- [int](#int) 🟡
- [double](#double) 🟡
- [string](#string) 🟡

### flow control

- [case](#case) 🟡
- [if](#if) 🟡
- [ifnull](#ifnull) 🟡
- [nullif](#nullif) 🟡

### mathematical

- [abs](#abs) 🟢
- [acos](#acos) 🟢
- [asin](#asin) 🟢
- [atan](#atan) 🟢
- [ceil](#ceil) 🟢
- [ceiling](#ceiling) 🟢
- [conv](#conv) 🟡
- [cos](#cos) 🟢
- [cot](#cot) 🟢
- [degrees](#degrees) 🟢
- [exp](#exp) 🟢
- [floor](#floor) 🟢
- [ln](#ln) 🟢
- [log](#log) 🟢
- [log2](#log2) 🟢
- [log10](#log10) 🟢
- [mod](#mod) 🟢
- [pi](#pi) 🟢
- [pow](#pow) 🟢
- [power](#power) 🟢
- [radians](#radians) 🟢
- [rand](#rand) 🟢
- [round](#round) 🟢
- [sign](#sign) 🟢
- [sin](#sin) 🟢
- [sqrt](#sqrt) 🟢
- [tan](#tan) 🟢
- [truncate](#truncate) 🟢

### date and time

- [adddate](#adddate) 🟡
- [addtime](#addtime) 🟡
- [curdate](#curdate) 🟡
- [current_date](#current_date) 🟡
- [current_time](#current_time) 🟡
- [current_timestamp](#current_timestamp) 🟡
- [curtime](#curtime) 🟡
- [date](#date) 🟡
- [datediff](#datediff) 🟡
- [data_add](#date_add) 🟡
- [date_sub](#date_sub) 🟡
- [day](#day) 🟡
- [dayname](#dayname) 🟡
- [dayofmonth](#dayofmonth) 🟡
- [dayofweek](#dayofweek) 🟡
- [dayofyear](#dayofyear) 🟡
- [from_unixtime](#from_unixtime) 🟡
- [hour](#hour) 🟡
- [last_day](#last_day) 🟡
- [localtime](#localtime) 🟡
- [localtimestamp](#localtimestamp) 🟡
- [makedate](#makedate) 🟡
- [maketime](#maketime) 🟡
- [minute](#minute) 🟡
- [month](#month) 🟡
- [monthname](#monthname) 🟡
- [now](#now) 🟡
- [quarter](#quarter) 🟡
- [second](#second) 🟡
- [sec_to_time](#sec_to_time) 🟡
- [subdate](#subdate) 🟡
- [subtime](#subtime) 🟡
- [sysdate](#sysdate) 🟡
- [time](#time) 🟡
- [timediff](#timediff) 🟡
- [time_to_sec](#time_to_sec) 🟡
- [unix_timestamp](#unix_timestamp) 🟡
- [week](#week) 🟡
- [weekday](#weekday) 🟡
- [weekofyear](#weekofyear) 🟡
- [year](#year) 🟡
- [yearweek](#yearweek) 🟡

### string

- [ascii](#ascii) 🟡
- [binary](#binary) 🟡
- [char](#char) 🟡
- [concat](#concat) 🟢
- [concat_ws](#concat_ws) 🟡
- [etl](#elt) 🟡
- [field](#field) 🟡
- [hex](#hex) 🟢
- [inser](#insert) 🟡
- [instr](#instr) 🟡
- [lcase](#lcase) 🟡
- [left](#left) 🟡
- [length](#length) 🟡
- [locate](#locate) 🟡
- [lower](#lower) 🟡
- [lpad](#lpad) 🟡
- [ltrim](#ltrim) 🟡
- [mid](#mid) 🟡
- [oct](#oct) 🟡
- [position](#position) 🟡
- [repeat](#repeat) 🟡
- [replace](#replace) 🟡
- [reverse](#reverse) 🟡
- [right](#right) 🟡
- [rpad](#rpad) 🟡
- [rtrim](#rtrim) 🟡
- [space](#space) 🟡
- [strcmp](#strcmp) 🟡
- [substr](#substr) 🟡
- [substring](#substring) 🟡
- [substring_index](#substring_index) 🟡
- [trim](#trim) 🟡
- [ucase](#ucase) 🟡
- [unhex](#unhex) 🟡
- [upper](#upper) 🟡

### hash

- [from_base64](#from_base64) 🟡
- [to_base64](#to_base64) 🟡
- [uuid](#uuid) 🟡

### aggregate

- [avg](#avg) 🟡
- [count](#count) 🟡
- [group_concat](#group_concat) 🟡
- [max](#max) 🟡
- [min](#min) 🟡
- [sum](#sum) 🟢

### system

- [app_name](#app_name) 🟢
- [author](#author) 🟢
- [version](#version) 🟢

### misc

- [coalese](#coalesce) 🟡
- [greatest](#greatest) 🟢
- [isnull](#isnull) 🟡
- [least](#least) 🟢
- [sleep](#sleep) 🟡

## Function description

### abs

> prototype: abs(`x`)

Returns the absolute value of `X`, or NULL if `X` is NULL.

### acos

> prototype: acos(`x`)


Returns the arc cosine of `X`, that is, the value whose cosine is `X`.
Returns NULL if `X` is not in the range -1 to 1, or if `X` is NULL.

### app_name

> prototype: app_name()

Returns the application name.

### asin

> prototype: asin(`x`)

Returns the arc sine of `X`, that is, the value whose sine is `X`.
Returns NULL if `X` is not in the range -1 to 1, or if `X` is NULL.

### atan

> prototype: atan(`x`)

Returns the arc tangent of `X`, that is, the value whose tangent is `X`.
Returns NULL if `X` is NULL.

### author

> prototype: author()

Returns the author of the application.

### ceil

> prototype: ceil(`x`)

ceil() is a synonym for `CEILING()`.

### ceiling

> prototype: ceiling(`x`)

Returns the smallest integer value not less than `X`. Returns NULL if `X` is NULL.

### concat

> prototype: concat(`str1`, `str2`, ...)

Returns the string that results from concatenating the arguments. May have one or more arguments.

Returns NULL if any argument is NULL.

### cos

> prototype: cos(`x`)

Returns the cosine of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### cot

> prototype: cot(`x`)

Returns the cotangent of `X`. Returns NULL if `X` is NULL.

### degrees

> prototype: degrees(`x`)

Returns the argument `X`, converted from radians to degrees. Returns NULL if `X` is NULL.

### exp

> prototype: exp(`x`)

Returns the value of e (the base of natural logarithms) raised to the power of `X`.

The inverse of this function is `LOG()` (using a single argument only) or `LN()`.

If `X` is NULL, this function returns NULL.

### floor

> prototype: floor(`x`)

Returns the largest integer value not greater than `X`. Returns NULL if `X` is NULL.

### greatest

> prototype: greatest(`x1`, `x2`, ...)

With two or more arguments, returns the largest (maximum-valued) argument.
The arguments are compared using the same rules as for `LEAST()`.

- If any argument is NULL, the result is NULL.
- If any argument is a string, the result is a string.
- If any argument is a real number, the result is a real number.
- If all arguments are integer-valued, the result is an integer.

Returns NULL if any argument is NULL.

### hex

> prototype: hex(`n` or `s`)

This function can be used to obtain a hexadecimal representation of a decimal number or a string.

### least

> prototype: least(`x1`, `x2`, ...)

With two or more arguments, returns the smallest (minimum-valued) argument.
The arguments are compared using the same rules as for `GREATEST()`.

### ln

> prototype: ln(`x`)

Returns the natural logarithm of `X`; that is, the base-e logarithm of `X`.

If `X` is less than or equal to 0, returns NULL.
Returns NULL if `X` is NULL.

This function is synonymous with LOG(`X`).

The inverse of this function is `EXP()`.

### log

> prototype: log(`[b]`, `x`)

If called with one parameter, this function returns the natural logarithm of `X`.
If `X` is less than or equal to 0, returns NULL.

Returns NULL if `X` or `B` is NULL.

The inverse of this function (when called with a single argument) is the `EXP()`.

If called with two parameters, this function returns the logarithm of `X` to the base `B`.

If `X` is less than or equal to 0, or if `B` is less than or equal to 1, then NULL is returned.

`LOG`(`B`, `X`) is equivalent to `LOG`(`X`) / `LOG`(`B`).

### log2

> prototype: log2(`x`)

Returns the base-2 logarithm of `X`. If `X` is less than or equal to 0, returns NULL.

Returns NULL if `X` is NULL.

This function is equivalent to the expression `LOG`(`X`) / `LOG`(2).

### log10

> prototype: log10(`x`)

Returns the base-10 logarithm of `X`. If `X` is less than or equal to 0, returns NULL.

Returns NULL if `X` is NULL.

`LOG10`(`X`) is equivalent to `LOG`(10, `X`).

### mod

> prototype: mod(`n`, `m`)

Modulo operation. Returns the remainder of `N` divided by `M`. Returns NULL if `M` or `N` is NULL.

`MOD`(`N`, 0) returns NULL.

### pi

> prototype: pi()

Returns the value of `π`(pi). The default number of decimal places displayed is 8,
but uses the full double-precision value internally.

### pow

> prototype: pow(`x`, `y`)

Returns the value of `X` raised to the power of `Y`. Returns NULL if `X` or `Y` is NULL.

### power

> prototype: power(`x`, `y`)

This is a synonym for `POW()`.

### radians

> prototype: radians(`x`)

Returns the argument `X`, converted from degrees to radians. (Note that `π` radians equals 180 degrees.)

Returns NULL if `X` is NULL.

### rand

> prototype: rand(`[n]`)

Returns a random floating-point value `v` in the range 0 <= `v` < 1.0.

If an integer argument `N` is specified, it is used as the seed value.
For equal argument values, `RAND`(`N`) returns the same value each time.

### round

> prototype: round(`x`, `[d]`)

Rounds the argument `X` to `D` decimal places.
`D` defaults to 0 if not specified.
`D` can be negative to cause `D` digits left of the decimal point of the value `X` to become zero.

### sign

> prototype: sign(`x`)

Returns the sign of the argument as -1, 0, or 1, depending on whether `X` is negative, zero, or positive.
Returns NULL if `X` is NULL.

### sin

> prototype: sin(`x`)

Returns the sine of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### sqrt

> prototype: sqrt(`x`)

Returns the square root of a non-negative number `X`. If `X` is NULL, the function returns NULL.

### sum

> prototype: sum(`expr`)

Returns the sum of `expr`(skip NULL value). If any expr is not a number, or all expr are NULL, returns NULL.

### tan

> prototype: tan(`x`)

Returns the tangent of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### truncate

> prototype: truncate(`x`, `[d]`)

Returns the number `X`, truncated to `D` decimal places.
If `D` is 0, the result has no decimal point or fractional part.
`D` can be negative to cause `D` digits left of the decimal point of the value `X` to become zero.
If `X` or `D` is NULL, returns NULL.

### version

> prototype: version()

Returns the version of the application.
