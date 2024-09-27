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
- pkg-config
- Lib: OpenSSL
- Lib: uuid

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

- [int](#int) 🟢
- [double](#double) 🟢
- [string](#string) 🟢

### flow control

- [case](#case) 🟢
- [decode](#decode) 🟢
- [if](#if) 🟢
- [ifnull](#ifnull) 🟢
- [nullif](#nullif) 🟢

### mathematical

- [abs](#abs) 🟢
- [acos](#acos) 🟢
- [asin](#asin) 🟢
- [atan](#atan) 🟢
- [ceil](#ceil) 🟢
- [ceiling](#ceiling) 🟢
- [conv](#conv) 🟢
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

> All date and time formats are compatible with
> the [`strftime`](https://en.cppreference.com/w/cpp/chrono/c/strftime) function.

- [adddate](#adddate) 🟢
- [addtime](#addtime) 🟢
- [curdate](#curdate) 🟢
- [current_date](#current_date) 🟢
- [current_time](#current_time) 🟢
- [current_timestamp](#current_timestamp) 🟢
- [curtime](#curtime) 🟢
- [date](#date) 🟢
- [datediff](#datediff) 🟢
- [date_add](#date_add) 🟢
- [date_format](#date_format) 🟢
- [date_sub](#date_sub) 🟢
- [day](#day) 🟢
- [dayname](#dayname) 🟢
- [dayofmonth](#dayofmonth) 🟢
- [dayofweek](#dayofweek) 🟢
- [dayofyear](#dayofyear) 🟢
- [from_unixtime](#from_unixtime) 🟢
- [hour](#hour) 🟢
- [last_day](#last_day) 🟢
- [localtime](#localtime) 🟢
- [localtimestamp](#localtimestamp) 🟢
- [makedate](#makedate) 🟢
- [maketime](#maketime) 🟢
- [minute](#minute) 🟢
- [month](#month) 🟢
- [monthname](#monthname) 🟢
- [now](#now) 🟢
- [quarter](#quarter) 🟢
- [second](#second) 🟢
- [sec_to_time](#sec_to_time) 🟢
- [str_to_date](#str_to_date) 🟢
- [subdate](#subdate) 🟢
- [subtime](#subtime) 🟢
- [sysdate](#sysdate) 🟢
- [time](#time) 🟢
- [timediff](#timediff) 🟢
- [time_to_sec](#time_to_sec) 🟢
- [unix_timestamp](#unix_timestamp) 🟢
- [week](#week) 🟢
- [weekday](#weekday) 🟢
- [weekofyear](#weekofyear) 🟢
- [year](#year) 🟢
- [yearweek](#yearweek) 🟢

### strings

- [ascii](#ascii) 🟢
- [bin](#bin) 🟢
- [char](#char) 🟢
- [concat](#concat) 🟢
- [concat_ws](#concat_ws) 🟢
- [elt](#elt) 🟢
- [field](#field) 🟢
- [hex](#hex) 🟢
- [insert](#insert) 🟢
- [instr](#instr) 🟢
- [lcase](#lcase) 🟢
- [left](#left) 🟢
- [length](#length) 🟢
- [locate](#locate) 🟢
- [lower](#lower) 🟢
- [lpad](#lpad) 🟢
- [ltrim](#ltrim) 🟢
- [mid](#mid) 🟢
- [oct](#oct) 🟢
- [position](#position) 🟢
- [repeat](#repeat) 🟢
- [replace](#replace) 🟢
- [reverse](#reverse) 🟢
- [right](#right) 🟢
- [rpad](#rpad) 🟢
- [rtrim](#rtrim) 🟢
- [space](#space) 🟢
- [strcmp](#strcmp) 🟢
- [substr](#substr) 🟢
- [substring](#substring) 🟢
- [substring_index](#substring_index) 🟢
- [trim](#trim) 🟢
- [ucase](#ucase) 🟢
- [unhex](#unhex) 🟢
- [upper](#upper) 🟢

### hash

- [from_base64](#from_base64) 🟢
- [md5](#md5) 🟢
- [serial](#serial) 🟢
- [sha](#sha) 🟢
- [sha1](#sha1) 🟢
- [sha2](#sha2) 🟢
- [sha224](#sha224) 🟢
- [sha256](#sha256) 🟢
- [sha384](#sha384) 🟢
- [sha512](#sha512) 🟢
- [to_base64](#to_base64) 🟢
- [uuid](#uuid) 🟢

### aggregate

- [avg](#avg) 🟢
- [count](#count) 🟢
- [group_concat](#group_concat) 🟢
- [max](#max) 🟢
- [min](#min) 🟢
- [sum](#sum) 🟢

### system

- [app](#app) 🟢
- [author](#author) 🟢
- [version](#version) 🟢

### misc

- [coalese](#coalesce) 🟢
- [greatest](#greatest) 🟢
- [isnull](#isnull) 🟢
- [least](#least) 🟢
- [sleep](#sleep) 🟢

## Function description

### abs

> prototype: abs(`x`)

Returns the absolute value of `X`, or NULL if `X` is NULL.

### acos

> prototype: acos(`x`)


Returns the arc cosine of `X`, that is, the value whose cosine is `X`.
Returns NULL if `X` is not in the range -1 to 1, or if `X` is NULL.

### adddate

> prototype: adddate(`date`, `interval`, `[unit]`)

ADDDATE() is a synonym for [DATE_ADD()](#date_add).

### addtime

> prototype: addtime(`date`, `expr`)

Adds `expr` to `date`. `expr` is a time expression.
Return NULL if `date` or `expr` are NULL.

### app

> prototype: app()

Returns the application name.

### ascii

> prototype: ascii(`str`)

Returns the numeric value of the leftmost character of the string `str`.
Returns 0 if `str` is the empty string. Returns NULL if `str` is NULL.

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

### avg

> prototype: avg(`expr`)

Returns the average value of `expr`(skip NULL value). If all `expr`s are NULL, returns NULL.

### bin

> prototype: bin(`N`)

Returns a string representation of the binary value of `N`. Returns NULL if `N` is NULL.

This is equivalent to `CONV(N, 10, 2)`.

### case

> prototype: case(`when1`, `then1`, ..., `whenN`, `thenN`, `else`)

If `when1` is true, returns `then1`.
Otherwise, if `when2` is true, returns `then2`, and so on.

If none of the `when` expressions are true, returns `else`.
If `else` is omitted, returns NULL.

### ceil

> prototype: ceil(`x`)

ceil() is a synonym for [CEILING()](#ceiling).

### ceiling

> prototype: ceiling(`x`)

Returns the smallest integer value not less than `X`. Returns NULL if `X` is NULL.

### char

> prototype: char(`N`, ...)

CHAR() interprets each argument N as an integer and returns a string
consisting of the characters given by the code values of those integers.
NULL values are skipped.

### coalesce

> prototype: coalesce(`expr1`, `expr2`, ...)

Returns the first non-NULL value in the list, or NULL if there are no non-NULL values.

### concat

> prototype: concat(`str1`, `str2`, ...)

Returns the string that results from concatenating the arguments. May have one or more arguments.

Returns NULL if any argument is NULL.

### concat_ws

> prototype: concat_ws(`sep`, `str1`, `str2`, ...)

Concatenate with separator and is a special form of CONCAT().
The first argument is the separator for the rest of the arguments.

The separator is added between the strings to be concatenated.
The separator can be a string, as can the rest of the arguments.
If the separator is NULL, the result is NULL.

### conv

> prototype: conv(`n`, `from_base`, `to_base`)

Returns a string representation of the number `N`, converted from base `from_base` to base `to_base`.

Returns NULL if any argument is NULL or `N` is a invalid number.

The argument `N` is interpreted as an integer, but may be specified as an integer or a string.

The minimum base is 2 and the maximum base is 36.

### cos

> prototype: cos(`x`)

Returns the cosine of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### cot

> prototype: cot(`x`)

Returns the cotangent of `X`. Returns NULL if `X` is NULL.

### count

> prototype: count(`expr`)

Returns a count of the number of non-NULL values of `expr` in the rows.

### curdate

> prototype: curdate()

Returns the current date as a value in 'YYYY-MM-DD' format.

### current_date

> prototype: current_date()

CURRENT_DATE() is a synonym for [CURDATE()](#curdate).

### current_time

> prototype: current_time()

CURRENT_TIME() is a synonym for [CURTIME()](#curtime).

### current_timestamp

> prototype: current_timestamp()

CURRENT_TIMESTAMP() is a synonym for [NOW()](#now).

### curtime

> curtime()

Returns the current time as a value in 'hh:mm:ss' format.

### date

> prototype: date(`expr`)

Extracts the date part of the date or datetime expression `expr`.
Returns NULL if `expr` is NULL.

### datediff

> prototype: datediff(`expr1`, `expr2`)

Returns `expr1` − `expr2` expressed as a value in days from one date to the other.
`expr1` and `expr2` are date or date-and-time expressions.
Only the date parts of the values are used in the calculation.

### date_add

> prototype: date_add(`date`, `interval`, `[unit]`)

The `date` argument specifies the starting date or datetime value,
and the `interval` argument specifies the interval value to be added
to the starting date, the `unit` argument specifies the units of the interval value.

Available units are:

- 1: second
- 2: minute
- 3: hour
- 4: day, default
- 5: week

Returns NULL if either `date` or `interval` is NULL.

### date_format

> prototype: date_format(`date`, `format`)

Formats the `date` value according to the `format` string.
If either argument is NULL, the function returns NULL.

### date_sub

> prototype: date_sub(`date`, `interval`, `[unit]`)

It's the same as [date_add](#date_add) but subtracts the interval from the `date`.

### day

> prototype: day(`date`)

DAY() is a synonym for [DAYOFMONTH()](#dayofmonth).

### dayname

> prototype: dayname(`date`)

Returns the name of the weekday for `date`. The language used for the name is
controlled by the value of the lc_time_names system variable.
Returns NULL if `date` is NULL.

### dayofmonth

> prototype: dayofmonth(`date`)

Returns the day of the month for `date`, in the range 1 to 31.
Returns NULL if `date` is NULL.

### dayofweek

> prototype: dayofweek(`date`)

Returns the weekday index for `date` (0 = Sunday, 1 = Monday, ... 6 = Saturday).
Returns NULL if `date` is NULL.

### dayofyear

> prototype: dayofyear(`date`)

Returns the day of the year for `date`, in the range 1 to 366.
Returns NULL if `date` is NULL.

### decode

> prototype: decode(`expr`, `value1`, `result1`, `value2`, `result2`, ..., [`default`])

If `expr` equals `value1`, returns `result1`.
Otherwise, if `expr` equals `value2`, returns `result2`, and so on.

If `expr` matches none of the values, returns `default`.
If `default` is omitted, returns NULL.

### degrees

> prototype: degrees(`x`)

Returns the argument `X`, converted from radians to degrees. Returns NULL if `X` is NULL.

### double

> prototype: double(`x`)

Cast `X` to double. Returns 0 if not a valid number.

### elt

> prototype: elt(`N`, `str1`, `str2`, ...)

ELT() returns the `N`th element of the list of strings:
`str1` if `N` = 1, `str2` if `N` = 2, and so on.
Returns NULL if `N` is less than 1, greater than the number of arguments, or NULL.

ELT() is the complement of [FIELD()](#field).

### exp

> prototype: exp(`x`)

Returns the value of e (the base of natural logarithms) raised to the power of `X`.

The inverse of this function is `LOG()` (using a single argument only) or `LN()`.

If `X` is NULL, this function returns NULL.

### field

> prototype: field(`str`, `str1`, `str2`, ...)

Returns the index (position) of the `str` in the subsequent string list.
Returns 0 if `str` is not found.

FIELD() is the complement of [ELT()](#elt).

### floor

> prototype: floor(`x`)

Returns the largest integer value not greater than `X`. Returns NULL if `X` is NULL.

### from_base64

> prototype: from_base64(`str`)

Takes a string encoded with the base-64 encoded rules
and returns the decoded result as a binary string.

The result is NULL if the argument is NULL or not a valid base-64 string.

### from_unixtime

> prototype: from_unixtime(`timestamp`, `[format]`)

Returns a representation of unix_timestamp as a character string value
with the format given by the `format` argument.
If `format` is omitted, the default format is '%F %T'.

### greatest

> prototype: greatest(`x1`, `x2`, ...)

With two or more arguments, returns the largest (maximum-valued) argument.
The arguments are compared using the same rules as for `LEAST()`.

- If any argument is NULL, the result is NULL.
- If any argument is a string, the result is a string.
- If any argument is a real number, the result is a real number.
- If all arguments are integer-valued, the result is an integer.

Returns NULL if any argument is NULL.

### group_concat

> prototype: group_concat(`expr`)

Returns a string result with the concatenated non-NULL values from a group.
It returns NULL if there are no non-NULL values.

### hex

> prototype: hex(`n` or `s`)

This function can be used to obtain a hexadecimal representation of a decimal number or a string.

### hour

> prototype: hour(`time`)

Returns the hour for time. The range of the return value is 0 to 23.
Returns NULL if `time` is NULL.

### if

> prototype: if(`condition`, `true_value`, `false_value`)

Returns `true_value` if `condition` is true, otherwise returns `false_value`.

### ifnull

> prototype: ifnull(`expr1`, `expr2`)

Returns `expr2` if `expr1` is NULL, otherwise returns `expr1`.
This is the same as the `COALESCE()` function with two arguments.

### insert

> prototype: insert(`str`, `pos`, `len`, `newstr`)

Returns the string `str`, with the substring beginning at position `pos`
and `len` characters long replaced by the string `newstr`.
Returns the original string if `pos` is not within the length of the string.
Replaces the rest of the string from position `pos` if `len` is not within
the length of the rest of the string.

Returns NULL if any argument is NULL.

### instr

> prototype: instr(`str`, `substr`)

Returns the position of the first occurrence of substring `substr` in string `str`.
This is the same as the two-argument form of [LOCATE()](#locate),
except that the order of the arguments is reversed.

### isnull

> prototype: isnull(`expr`)

If `expr` is NULL, ISNULL() returns `true`, otherwise it returns `false`.

### int

> prototype: int(`x`)

Cast `X` to integer. Returns 0 if not a valid number.

### last_day

> prototype: last_day(`date`)

Takes a date or datetime value and returns the corresponding value for the last day of the month.
Returns NULL if the `date` is invalid or NULL.

### lcase

> prototype: lcase(`str`)

LCASE() is a synonym for [LOWER()](#lower).

### least

> prototype: least(`x1`, `x2`, ...)

With two or more arguments, returns the smallest (minimum-valued) argument.
The arguments are compared using the same rules as for `GREATEST()`.

### left

> prototype: left(`str`, `len`)

Returns the leftmost `len` characters from the string `str`,
or NULL if any argument is NULL.

### length

> prototype: length(`str`)

Returns the length of the string str. Returns NULL if str is NULL.

### ln

> prototype: ln(`x`)

Returns the natural logarithm of `X`; that is, the base-e logarithm of `X`.

If `X` is less than or equal to 0, returns NULL.
Returns NULL if `X` is NULL.

This function is synonymous with LOG(`X`).

The inverse of this function is `EXP()`.

### localtime

> prototype: localtime()

LOCALTIME() is a synonym for [NOW()](#now).

### localtimestamp

> prototype: localtimestamp()

LOCALTIMESTAMP() is a synonym for [NOW()](#now).

### locate

> prototype: locate(`substr`, `str`, `[pos]`)

Returns the position of the first occurrence of substring `substr` in string `str`,
starting at position `pos`, when `pos` is omitted, it starts at position 1.

Returns 0 if `substr` is not in `str`. Returns NULL if any argument is NULL.

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

### lower

> prototype: lower(`str`)

Returns the string `str` with all characters changed to lowercase, or NULL if `str` is NULL.

### lpad

> prototype: lpad(`str`, `len`, `padstr`)

Returns the string `str`, left-padded with the string `padstr` to a length of `len` characters.
If `str` is longer than `len`, the return value is shortened to `len` characters.

Returns NULL if any of its arguments are NULL.

### ltrim

> prototype: ltrim(`str`)

Returns the string `str` with leading space characters removed.
Returns NULL if `str` is NULL.

### makedate

> prototype: makedate(`year`, `dayofyear`)

Returns a date, given year and day-of-year values.
`dayofyear` must be greater than 0 or the result is NULL.
The result is also NULL if either argument is NULL.

### maketime

> prototype: maketime(`hour`, `minute`, `second`)

Returns a time value calculated from the `hour`, `minute`, and `second` arguments.
Returns NULL if any of its arguments are NULL.

### max

> prototype: max(`expr`)

Returns the maximum value of `expr`.
MAX() may take a string argument; in such cases, it returns the maximum string value.

### md5

> prototype: md5(`str`)

Calculates an MD5 128-bit checksum for the string.
The value is returned as a string of 32 hexadecimal digits,
or NULL if the argument was NULL.

### mid

> prototype: mid(`str`, `pos`, `[len]`)

MID() is a synonym for [SUBSTRING()](#substring).

### min

> prototype: min(`expr`)

Returns the minimum value of `expr`.
MIN() may take a string argument; in such cases, it returns the minimum string value.

### minute

> prototype: minute(`time`)

Returns the minute for `time`, in the range 0 to 59, or NULL if time is NULL.

### mod

> prototype: mod(`n`, `m`)

Modulo operation. Returns the remainder of `N` divided by `M`. Returns NULL if `M` or `N` is NULL.

`MOD`(`N`, 0) returns NULL.

### month

> prototype: month(`date`)

Returns the month for `date`, in the range 1 to 12 for January to December.
Returns NULL if date is NULL.

### monthname

> prototype: monthname(`date`)

Returns the full name of the month for `date`. The language used for the name is
controlled by the value of the lc_time_names system variable.
Returns NULL if `date` is NULL.

### now

> prototype: now()

Returns the current date and time as a value in 'YYYY-MM-DD hh:mm:ss' format.

### nullif

> prototype: nullif(`expr1`, `expr2`)

Returns NULL if `expr1` equals `expr2`; otherwise returns `expr1`.
This is the same as IF(`expr1` = `expr2`, NULL, `expr1`).

### oct

> prototype: oct(`N`)

Returns a string representation of the octal value of `N`.
This is equivalent to `CONV(N, 10, 8)`. Returns NULL if `N` is NULL.

### pi

> prototype: pi()

Returns the value of `π`(pi).

### position

> prototype: position(`substr`, `str`)

This is the same as the two-argument form of [LOCATE()](#locate)

### pow

> prototype: pow(`x`, `y`)

Returns the value of `X` raised to the power of `Y`. Returns NULL if `X` or `Y` is NULL.

### power

> prototype: power(`x`, `y`)

This is a synonym for [POW()](#pow).

### quarter

> prototype: quarter(`date`)

Returns the quarter of the year for `date`,
in the range 1 to 4, or NULL if `date` is NULL.

### radians

> prototype: radians(`x`)

Returns the argument `X`, converted from degrees to radians. (Note that `π` radians equals 180 degrees.)

Returns NULL if `X` is NULL.

### rand

> prototype: rand(`[n]`)

Returns a random floating-point value `v` in the range 0 <= `v` < 1.0.

If an integer argument `N` is specified, it is used as the seed value.
For equal argument values, `RAND`(`N`) returns the same value each time.

### repeat

> prototype: repeat(`str`, `count`)

Returns a string consisting of the string `str` repeated `count` times.
If `count` is less than 1, returns an empty string.
Returns NULL if `str` or `count` is NULL.

### replace

> prototype: replace(`str`, `from`, `to`)

Returns the string `str` with
all occurrences of the string `from` replaced by the string `to`.
This function is case sensitive.

Returns NULL if any of its arguments are NULL.

### reverse

> prototype: reverse(`str`)

Returns the string `str` with the order of the characters reversed,
or NULL if `str` is NULL.

### right

> prototype: right(`str`, `len`)

Returns the rightmost `len` characters from the string `str`,
or NULL if any argument is NULL.

### round

> prototype: round(`x`, `[d]`)

Rounds the argument `X` to `D` decimal places.
`D` defaults to 0 if not specified.
`D` can be negative to cause `D` digits left of the decimal point of the value `X` to become zero.

If `X` or `D` is NULL, returns NULL.

### rpad

> prototype: rpad(`str`, `len`, `padstr`)

Returns the string `str`, right-padded with the string `padstr` to a length of `len` characters.
If `str` is longer than `len`, the return value is shortened to `len` characters.

Returns NULL if any of its arguments are NULL.

### rtrim

> prototype: rtrim(`str`)

Returns the string `str` with trailing space characters removed.
Returns NULL if `str` is NULL.

### second

> prototype: second(`time`)

Returns the second for `time`, in the range 0 to 59, or NULL if `time` is NULL.

### sec_to_time

> prototype: sec_to_time(`seconds`)

Returns the `seconds` argument, converted to hours, minutes, and seconds.
Returns NULL if `seconds` is NULL.

### serial

> prototype: serial([`n`])

Returns a random serial number.
If an integer argument `N` is specified, it is used as the length of the serial number,
Otherwise, the length is 8.

### sha

> prototype: sha(`str`)

sha() is synonymous with [SHA1()](#sha1).

### sha1

> prototype: sha1(`str`)

Calculates an SHA-1 160-bit checksum for the string.
The value is returned as a string of 40 hexadecimal digits,
or NULL if the argument is NULL.

### sha2

> prototype: sha2(`str`, `[n]`)

Calculates the SHA-2 family of hash functions (SHA-224, SHA-256, SHA-384, and SHA-512).
The first argument is the plaintext string to be hashed.
The second argument indicates the desired bit length of the result,
which must have a value of 224, 256, 384, 512, or 0 (which is equivalent to 256).
If the second argument is omitted, the default is 256.

If either argument is NULL or the hash length is not one of the permitted values,
the return value is NULL.
Otherwise, the function result is a hash value containing the desired number of bits.

### sha224

> prototype: sha224(`str`)

This function is the same as [SHA2()](#sha2) with a hash length of 224 bits.

### sha256

> prototype: sha256(`str`)

This function is the same as [SHA2()](#sha2) with a hash length of 256 bits.

### sha384

> prototype: sha384(`str`)

This function is the same as [SHA2()](#sha2) with a hash length of 384 bits.

### sha512

> prototype: sha512(`str`)

This function is the same as [SHA2()](#sha2) with a hash length of 512 bits.

### space

> prototype: space(`n`)

Returns a string consisting of `N` space characters, or NULL if `N` is NULL.

### sign

> prototype: sign(`x`)

Returns the sign of the argument as -1, 0, or 1, depending on whether `X` is negative, zero, or positive.
Returns NULL if `X` is NULL.

### sin

> prototype: sin(`x`)

Returns the sine of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### sleep

> prototype: sleep(`milliseconds`)

Sleep(pauses) for the specified number of milliseconds. Returns 0.

Note that since the program is single-threaded, the sleep function will
execute multiple times if there are multiple lines.

### sqrt

> prototype: sqrt(`x`)

Returns the square root of a non-negative number `X`. If `X` is NULL, the function returns NULL.

### strcmp

> prototype: strcmp(`str1`, `str2`)

Returns 0 if the strings are the same,
-1 if the first argument is smaller than the second, 1 otherwise.

Returns NULL if either argument is NULL.

### string

> prototype: string(`x`)

Cast `X` to string.

### str_to_date

> prototype: str_to_date(`str`, `format`)

Returns the argument parsed as a date. If `str` or `format` is NULL, the function returns NULL.
This function is the opposite of [DATE_FORMAT()](#date_format).

### subdate

> prototype: subdate(`date`, `days`)

SUBDATE() is a synonym for [DATE_SUB()](#date_sub) of two-argument form.

### substr

> prototype: substr(`str`, `pos`, `[len]`)

SUBSTR() is a synonym for [SUBSTRING()](#substring).

### substring

> prototype: substring(`str`, `pos`, `[len]`)

Returns a substring `len` characters long from string `str`, starting at position `pos`.
The first character of the string is at position 1.
It is also possible to use a negative value for pos.
In this case, the beginning of the substring is
pos characters from the end of the string, rather than the beginning.
A value of 0 for `pos` returns an empty string.

It returns NULL if any of its arguments are NULL.
If `len` is less than 1, the result is the empty string.

### substring_index

> prototype: substring_index(`str`, `delim`, `count`)

Returns the substring from string `str` before count occurrences of the delimiter `delim`.
If `count` is positive, everything to the left of the final delimiter (counting from the left) is returned.
If `count` is negative, everything to the right of the final delimiter (counting from the right) is returned.

Returns NULL if any of its arguments are NULL.

### subtime

> prototype: subtime(`date`, `expr`)

It's the same as [ADDTIME()](#addtime) but subtracts.

### sum

> prototype: sum(`expr`)

Returns the sum of `expr`(skip NULL value). If all `expr`s are NULL, returns NULL.

### sysdate

> prototype: sysdate()

It's a synonym for [NOW()](#now).

### tan

> prototype: tan(`x`)

Returns the tangent of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### time

> prototype: time(`expr`)

Extracts the time part of the time or datetime expression `expr` and
returns it as a string. Returns NULL if `expr` is NULL.

### timediff

> prototype: timediff(`time1`, ``)

Returns time1 − time2 expressed as a time value.
Returns NULL if either argument is NULL.

### time_tosec

> prototype: time_tosec(`time`)

Returns the `time` argument, converted to seconds.
Returns NULL if time is NULL.

### to_base64

> prototype: to_base64(`str`)

Converts the string argument to base-64 encoded form and returns the result as
a character string with the connection character set and collation.
If the argument is not a string, it is converted to a string
before conversion takes place.

The result is NULL if the argument is NULL.
Base-64 encoded strings can be decoded using the [FROM_BASE64()](#from_base64) function.

### trim

> prototype: trim(`str`)

Returns the string `str` with all leading or trailing space characters removed.
Returns NULL if `str` is NULL.

### truncate

> prototype: truncate(`x`, `[d]`)

Returns the number `X`, truncated to `D` decimal places.
If `D` is 0, the result has no decimal point or fractional part.
`D` can be negative to cause `D` digits left of the decimal point of the value `X` to become zero.

If `X` or `D` is NULL, returns NULL.

## ucase

> prototype: ucase(`str`)

UCASE() is a synonym for [UPPER()](#upper).

### unhex

> prototype: unhex(`hex_string`)

Returns a string containing the character representation of the hexadecimal argument.

The characters in the argument string must be legal hexadecimal digits.
If the argument contains any non-hexadecimal digits,
or is itself NULL, the result is NULL:

It's the opposite of the [HEX()](#hex) function.
This is different from the function of the same name in MySQL.

### unix_timestamp

> prototype: unix_timestamp(`[date]`)

Returns the value of the argument as seconds since '1970-01-01 00:00:00' UTC.
If `date` is omitted, use the current date and time.

### upper

> prototype: upper(`str`)

Returns the string `str` with all characters changed to uppercase, or NULL if `str` is NULL.

### uuid

> prototype: uuid()

Returns a string that conforms to UUID version 4 as described in RFC 4122.

### version

> prototype: version()

Returns the version of the application.

### week

> prototype: week(`date`, `[first_day]`, `[mode]`)

This function returns the week number for `date`.
The `first_day` argument determines the first day of the week,
and the `mode` argument determines the counting mode of the weeks.
The default values for `first_day` and `mode` are 1 and 3, respectively.

`first_day` can be 0 (Sunday) or 1 (Monday), ... 6 (Saturday).
`mode` can be one of the following values:

- 0: in range 0-53, week 0 is the week that with a `first_day` in this year.
- 1: in range 0-53, week 0 is the week that with 4 or more days in this year.
- 2: in range 1-53, week 1 is the week that with a `first_day` in this year.
- 3: in range 1-53, week 1 is the week that with 4 or more days in this year.

### weekday

> prototype: weekday(`date`)

Returns the weekday index for `date` (0 = Sunday, 1 = Monday, ... 6 = Saturday).
Returns NULL if date is NULL.

### weekofyear

> prototype: weekofyear(`date`)

Returns the calendar week of the `date` as a number in the range from 1 to 53.
Returns NULL if `date` is NULL.

This function is equivalent to WEEK(date, 1, 3).

### year

> prototype: year(`date`)

Returns the year for `date`.
The year is returned as a number in the range 1000 to 9999.
Returns NULL if `date` is NULL.

### yearweek

> prototype: yearweek(`date`, `[first_day]`, `[mode]`)

Returns the year and week for `date`.
The year in the result may be different from the year in the `date` argument
for the first and the last week of the year. Returns NULL if `date` is NULL.

The `first_day` and `mode` argument works exactly like to [WEEK()](#week).
