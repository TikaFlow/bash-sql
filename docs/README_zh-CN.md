# 基础用法

```bash
sql [选项] [查询语句]
```

> 查询语句必须用双引号括起来。

## 选项

- `-h`, `--help`：显示此帮助信息并退出。
- `-v`, `--version`：输出版本信息并退出。
- `-t`, `--title`：输出标题。
- `-l`, `--line-no`：输出行号。
- `-f`, `--file=FILE`：从文件中读取数据。
- `-d`, `--delimiter=DELIMITER`：指定字符作为字段分隔符。
- `-c`, `--columns=COLUMNS`：指定数据源的列数。

## 查询语句

多条查询语句使用`|`分割，每条查询语句结构如下：

```
select COLUMNS [WHERE] [ORDER BY] [LIMITS]
```

> 查询语句的关键字大小写不敏感。

- `COLUMNS`：选择列的列表。使用`*`选择所有列。默认列名为`col1`,`col2`, ...。可以使用`as`进行列别名。
- `WHERE`：使用`like`或`reg`子句过滤行。使用`not`否定`like`或`reg`。
    - 当使用`like`时，支持`%`和`_`，与`MySQL`中的用法相同，可以使用`\`转义。
    - 当使用`reg`时，模式应为正则表达式。
- `ORDER BY`：根据列重新排序数据。指定一个或多个列。使用`asc`表示升序，使用`desc`表示降序。
- `LIMITS`：限制输出行数。可以使用`limit lines`或`limit offset, lines`。

## 示例

```bash
ps -aux | sql -tlc11 "select *, col1 as user, col2 as pid, col9 as start, col11 as command \
  where col2 not like PID | select * order by start desc limit 10"
```

# 支持的函数

> ☄️ 待实现
>
> ✔️ 已实现

## 类型转换/type conversion

## 流程控制/flow control

## 数学/math

### abs(`x`) ☄️

Returns the absolute value of `X`, or NULL if `X` is NULL.

### acos(`x`) ☄️

Returns the arc cosine of `X`, that is, the value whose cosine is `X`.
Returns NULL if `X` is not in the range -1 to 1, or if `X` is NULL.

### asin(`x`) ☄️

Returns the arc sine of `X`, that is, the value whose sine is `X`.
Returns NULL if `X` is not in the range -1 to 1, or if `X` is NULL.

### atan(`x`) ☄️

Returns the arc tangent of `X`, that is, the value whose tangent is `X`.
Returns NULL if `X` is NULL.

### ceil(`x`) ☄️

ceil() is a synonym for `CEILING()`.

### ceiling(`x`) ☄️

Returns the smallest integer value not less than `X`. Returns NULL if `X` is NULL.

### cos(`x`) ☄️

Returns the cosine of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### cot(`x`) ☄️

Returns the cotangent of `X`. Returns NULL if `X` is NULL.

### degrees(`x`) ☄️

Returns the argument `X`, converted from radians to degrees. Returns NULL if `X` is NULL.

### exp(`x`) ☄️

Returns the value of e (the base of natural logarithms) raised to the power of `X`.

The inverse of this function is `LOG()` (using a single argument only) or `LN()`.

If `X` is NULL, this function returns NULL.

### floor(`x`) ☄️

Returns the largest integer value not greater than `X`. Returns NULL if `X` is NULL.

### greatest(`x1`, `x2`, ...) ☄️

With two or more arguments, returns the largest (maximum-valued) argument.
The arguments are compared using the same rules as for `LEAST()`.

- If any argument is NULL, the result is NULL.
- If any argument is a string, the result is a string.
- If any argument is a real number, the result is a real number.
- If all arguments are integer-valued, the result is an integer.

Returns NULL if any argument is NULL.

### hex(`n` or `s`) ☄️

This function can be used to obtain a hexadecimal representation of a decimal number or a string.

### least(`x1`, `x2`, ...) ☄️

With two or more arguments, returns the smallest (minimum-valued) argument.
The arguments are compared using the same rules as for `GREATEST()`.

### ln(`x`) ☄️

Returns the natural logarithm of `X`; that is, the base-e logarithm of `X`.

If `X` is less than or equal to 0, returns NULL.
Returns NULL if `X` is NULL.

This function is synonymous with LOG(`X`).

The inverse of this function is `EXP()`.

### log(`[b]`, `x`) ☄️

If called with one parameter, this function returns the natural logarithm of `X`.
If `X` is less than or equal to 0, returns NULL.

Returns NULL if `X` or `B` is NULL.

The inverse of this function (when called with a single argument) is the `EXP()`.

If called with two parameters, this function returns the logarithm of `X` to the base `B`.

If `X` is less than or equal to 0, or if `B` is less than or equal to 1, then NULL is returned.

`LOG`(`B`, `X`) is equivalent to `LOG`(`X`) / `LOG`(`B`).

### log2(`x`) ☄️

Returns the base-2 logarithm of `X`. If `X` is less than or equal to 0, returns NULL.

Returns NULL if `X` is NULL.

This function is equivalent to the expression `LOG`(`X`) / `LOG`(2).

### log10(`x`) ☄️

Returns the base-10 logarithm of `X`. If `X` is less than or equal to 0, returns NULL.

Returns NULL if `X` is NULL.

`LOG10`(`X`) is equivalent to `LOG`(10, `X`).

### mod(`n`, `m`) ✔️

Modulo operation. Returns the remainder of `N` divided by `M`. Returns NULL if `M` or `N` is NULL.

`MOD`(`N`, 0) returns NULL.

### pi() ☄️

Returns the value of `π`(pi). The default number of decimal places displayed is 8,
but uses the full double-precision value internally.


### pow(`x`, `y`) ☄️

Returns the value of `X` raised to the power of `Y`. Returns NULL if `X` or `Y` is NULL.

### power(`x`, `y`) ☄️

This is a synonym for `POW()`.

### radians(`x`) ☄️

Returns the argument `X`, converted from degrees to radians. (Note that `π` radians equals 180 degrees.)

Returns NULL if `X` is NULL.

### rand(`[n]`) ☄️

Returns a random floating-point value `v` in the range 0 <= `v` < 1.0.

If an integer argument `N` is specified, it is used as the seed value.
For equal argument values, `RAND`(`N`) returns the same value each time.

### round(`x`, `[d]`) ☄️

Rounds the argument `X` to `D` decimal places.
`D` defaults to 0 if not specified.
`D` can be negative to cause `D` digits left of the decimal point of the value `X` to become zero.

### sign(`x`) ☄️

Returns the sign of the argument as -1, 0, or 1, depending on whether `X` is negative, zero, or positive.
Returns NULL if `X` is NULL.

### sin(`x`) ☄️

Returns the sine of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### sqrt(`x`) ☄️

Returns the square root of a non-negative number `X`. If `X` is NULL, the function returns NULL.

### tan(`x`) ☄️

Returns the tangent of `X`, where `X` is given in radians. Returns NULL if `X` is NULL.

### truncate(`x`, `[d]`) ☄️

Returns the number `X`, truncated to `D` decimal places.
If `D` is 0, the result has no decimal point or fractional part.
`D` can be negative to cause `D` digits left of the decimal point of the value `X` to become zero.
If `X` or `D` is NULL, returns NULL.

## 日期时间/date and time

## 字符串/string

### concat(`str1`, `str2`, ...) ✔️

Returns the string that results from concatenating the arguments. May have one or more arguments.

Returns NULL if any argument is NULL.

## 散列/hash

## 聚合/aggregate

### sum(`expr`) ✔️

Returns the sum of `expr`(skip NULL value). If any expr is not a number, or all expr are NULL, returns NULL.

## 其他/misc