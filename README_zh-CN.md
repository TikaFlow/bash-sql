# Bash-Select

在bash中使用类似SQL的语言处理轻量级数据。

❗ **请勿在生产环境中使用此应用**，因为：

❕ 此应用所有的数字类型都被视为“double”（可能会丢失精度），并且

❕ 未经性能测试，仅推荐用于处理轻量级数据。

# 安装

## #0. 环境需求

- cmake
- make
- g++
- pkg-config
- Lib: OpenSSL
- Lib: uuid

## #1. 通过克隆项目或其他方式下载源代码

## #2. 进入仓库目录，创建并进入`build`目录

```bash
cd bash-sql
mkdir build && cd build
```

## #3. 生成Makefile并配置安装位置

```bash
cmake -DCMAKE_INSTALL_PREFIX=/your/custom/prefix ..
```

## #4. 编译并安装

```bash
make && make install
```

# 使用方法

```bash
sql [选项] [查询语句]
```

> 查询语句必须用单引号括起来。

## 命令行选项

- `-h`, `--help`：显示此帮助信息并退出。
- `-v`, `--version`：输出版本信息并退出。
- `-t`, `--title`：首行是表头。
- `-l`, `--line-no`：输出行号。
- `-i`, `--interactive`: 交互模式。
- `-f`, `--file=FILE`：从文件中读取数据。
- `-d`, `--delimiter=DELIMITER`：指定字符作为字段分隔符。
- `-c`, `--columns=COLUMNS`：指定数据源的列数。

## SQL语句

支持的语句：

```
create: CREATE TABLE {table_name} AS {select}
```

```
insert: INSERT INTO {table_name}[column_list] {VALUES {values_list} | {select}}
```

```
update: UPDATE {table_name} SET {column}={value} [WHERE]
```

```
select: [WITH] select {columns} [FROM] [WHERE] [GROUP BY] [ORDER BY] [LIMITS]
with: WITH {table_name} AS {select}
```

```
delete: DELETE FROM {table_name} [WHERE]
```

```
drop: DROP TABLE {table_name}
```

```
describe: DESC|DESCRIBE {table_name}
```

```
show: SHOW TABLES
```

```bash
# 非SQL语句末尾不需要分号
history # 显示历史记录

!n # 重复执行历史记录中第n条命令
```

> 关键字和函数不区分大小写。

语法大体上与标准`SQL`一致，但：

- 输入数据被放置在一个名为“std”的表中。
- 不支持子查询，请改用`with`。
- Group by只允许字段名或字段索引（在select子句中）。

## 示例

```bash
ps -aux | ./sql -tlc11 'select user, pid, `%cpu`, `%mem`, command from std limit 10;'
```

## 支持的函数

### 类型转换

- [int](#int)
- [double](#double)
- [string](#string)

### 流程控制

- [case](#case)
- [decode](#decode)
- [if](#if)
- [ifnull](#ifnull)
- [nullif](#nullif)

### 数学计算

- [abs](#abs)
- [acos](#acos)
- [asin](#asin)
- [atan](#atan)
- [ceil](#ceil)
- [ceiling](#ceiling)
- [conv](#conv)
- [cos](#cos)
- [cot](#cot)
- [degrees](#degrees)
- [exp](#exp)
- [floor](#floor)
- [ln](#ln)
- [log](#log)
- [log2](#log2)
- [log10](#log10)
- [mod](#mod)
- [pi](#pi)
- [pow](#pow)
- [power](#power)
- [radians](#radians)
- [rand](#rand)
- [round](#round)
- [sign](#sign)
- [sin](#sin)
- [sqrt](#sqrt)
- [tan](#tan)
- [truncate](#truncate)

### 日期时间

> 所有的日期/时间格式化字符串与
> [`strftime`](https://en.cppreference.com/w/cpp/chrono/c/strftime)函数兼容。

- [adddate](#adddate)
- [addtime](#addtime)
- [curdate](#curdate)
- [current_date](#current_date)
- [current_time](#current_time)
- [current_timestamp](#current_timestamp)
- [curtime](#curtime)
- [date](#date)
- [datediff](#datediff)
- [date_add](#date_add)
- [date_format](#date_format)
- [date_sub](#date_sub)
- [day](#day)
- [dayname](#dayname)
- [dayofmonth](#dayofmonth)
- [dayofweek](#dayofweek)
- [dayofyear](#dayofyear)
- [from_unixtime](#from_unixtime)
- [hour](#hour)
- [last_day](#last_day)
- [localtime](#localtime)
- [localtimestamp](#localtimestamp)
- [makedate](#makedate)
- [maketime](#maketime)
- [minute](#minute)
- [month](#month)
- [monthname](#monthname)
- [now](#now)
- [quarter](#quarter)
- [second](#second)
- [sec_to_time](#sec_to_time)
- [str_to_date](#str_to_date)
- [subdate](#subdate)
- [subtime](#subtime)
- [sysdate](#sysdate)
- [time](#time)
- [timediff](#timediff)
- [time_to_sec](#time_to_sec)
- [unix_timestamp](#unix_timestamp)
- [week](#week)
- [weekday](#weekday)
- [weekofyear](#weekofyear)
- [year](#year)
- [yearweek](#yearweek)

### 字符串

- [ascii](#ascii)
- [bin](#bin)
- [char](#char)
- [concat](#concat)
- [concat_ws](#concat_ws)
- [elt](#elt)
- [field](#field)
- [hex](#hex)
- [insert](#insert)
- [instr](#instr)
- [lcase](#lcase)
- [left](#left)
- [length](#length)
- [locate](#locate)
- [lower](#lower)
- [lpad](#lpad)
- [ltrim](#ltrim)
- [mid](#mid)
- [oct](#oct)
- [position](#position)
- [repeat](#repeat)
- [replace](#replace)
- [reverse](#reverse)
- [right](#right)
- [rpad](#rpad)
- [rtrim](#rtrim)
- [space](#space)
- [strcmp](#strcmp)
- [substr](#substr)
- [substring](#substring)
- [substring_index](#substring_index)
- [trim](#trim)
- [ucase](#ucase)
- [unhex](#unhex)
- [upper](#upper)

### 散列

- [from_base64](#from_base64)
- [md5](#md5)
- [serial](#serial)
- [sha](#sha)
- [sha1](#sha1)
- [sha2](#sha2)
- [sha224](#sha224)
- [sha256](#sha256)
- [sha384](#sha384)
- [sha512](#sha512)
- [to_base64](#to_base64)
- [uuid](#uuid)

### 聚合

- [avg](#avg)
- [count](#count)
- [group_concat](#group_concat)
- [max](#max)
- [min](#min)
- [sum](#sum)

### 系统

- [app](#app)
- [author](#author)
- [export](#export)
- [get](#get)
- [import](#import)
- [set](#set)
- [unset](#unset)
- [version](#version)

### 其他

- [coalese](#coalesce)
- [greatest](#greatest)
- [isnull](#isnull)
- [least](#least)
- [sleep](#sleep)

## 函数描述

### abs

> 原型：abs(`x`)

返回 `x` 的绝对值，如果 `x` 是 NULL，则返回 NULL。

### acos

> 原型：acos(`x`)

返回 `x` 的反余弦值，即余弦值为 `x` 的值。如果 `x` 不在 -1 到 1 的范围内，或者 `x` 是 NULL，则返回 NULL。

### adddate

> 原型：adddate(`date`, `interval`, `[unit]`)

ADDDATE() 是 [DATE_ADD()](#date_add) 的同义词。

### addtime

> 原型：addtime(`date`, `expr`)

将 `expr` 添加到 `date`。`expr` 是一个时间表达式。如果 `date` 或 `expr` 是 NULL，则返回 NULL。

### app

> 原型：app()

返回应用程序的名称。

### ascii

> 原型：ascii(`str`)

返回字符串 `str` 最左边字符的 ASCII 值。如果 `str` 是空字符串，则返回 0。如果 `str` 是 NULL，则返回 NULL。

### asin

> 原型：asin(`x`)

返回 `x` 的反正弦值，即正弦值为 `x` 的值。如果 `x` 不在 -1 到 1 的范围内，或者 `x` 是 NULL，则返回 NULL。

### atan

> 原型：atan(`x`)

返回 `x` 的反正切值，即正切值为 `x` 的值。如果 `x` 是 NULL，则返回 NULL。

### author

> 原型：author()

返回应用程序的作者。

### avg

> 原型：avg(`expr`)

返回 `expr` 的平均值（跳过 NULL 值）。如果所有的 `expr` 都是 NULL，则返回 NULL。

### bin

> 原型：bin(`n`)

返回 `n` 的二进制值的字符串表示。如果 `n` 是 NULL，则返回 NULL。

这等同于 `CONV(N, 10, 2)`。

### case

> 原型：case(`when1`, `then1`, ..., `whenN`, `thenN`, `else`)

如果 `when1` 为真，返回 `then1`。否则，如果 `when2` 为真，返回 `then2`，依此类推。

如果所有的 `when` 表达式都不为真，返回 `else`。如果省略了 `else`，则返回 NULL。

### ceil

> 原型：ceil(`x`)

ceil() 是 [CEILING()](#ceiling) 的同义词。

### ceiling

> 原型：ceiling(`x`)

返回不小于 `x` 的最小整数值。如果 `x` 是 NULL，则返回 NULL。

### char

> 原型：char(`n`, ...)

将每个参数 `n` 解释为整数，并返回由这些整数的代码值给出的字符组成的字符串。跳过 NULL 值。

### coalesce

> 原型：coalesce(`expr1`, `expr2`, ...)

返回列表中第一个非 NULL 值，或者如果没有非 NULL 值，则返回 NULL。

### concat

> 原型：concat(`str1`, `str2`, ...)

返回通过连接参数得到的字符串。可以有一个或多个参数。如果任何参数是 NULL，则返回 NULL。

### concat_ws

> 原型：concat_ws(`sep`, `str1`, `str2`, ...)

CONCAT() 的特殊形式。第一个参数是其余参数的分隔符。

分隔符被添加到要连接的字符串之间。分隔符可以是字符串，其余参数也可以是字符串。如果分隔符是 NULL，则结果为 NULL。

### conv

> 原型：conv(`n`, `from_base`, `to_base`)

返回数字 `n` 的字符串表示，从基数 `from_base` 转换为基数 `to_base`。

如果任何参数是 NULL 或 `n` 是一个无效数字，则返回 NULL。

参数 `n` 被解释为整数，但可以指定为整数或字符串。

基数的最小值是 2，最大值是 36。

### cos

> 原型：cos(`x`)

返回 `x` 的余弦值，其中 `x` 以弧度给出。如果 `x` 是 NULL，则返回 NULL。

### cot

> 原型：cot(`x`)

返回 `x` 的余切值。如果 `x` 是 NULL，则返回 NULL。

### count

> 原型：count(`expr`)

返回行中 `expr` 非 NULL 值的数量。

### curdate

> 原型：curdate()

以 'YYYY-mm-dd' 格式返回当前日期。

### current_date

> 原型：current_date()

CURRENT_DATE() 是 [CURDATE()](#curdate) 的同义词。

### current_time

> 原型：current_time()

CURRENT_TIME() 是 [CURTIME()](#curtime) 的同义词。

### current_timestamp

> 原型：current_timestamp()

CURRENT_TIMESTAMP() 是 [NOW()](#now) 的同义词。

### curtime

> curtime()

以 'HH:MM:SS' 格式返回当前时间。

### date

> 原型：date(`expr`)

提取日期或日期时间表达式 `expr` 的日期部分。如果 `expr` 是 NULL，则返回 NULL。

### datediff

> 原型：datediff(`expr1`, `expr2`)

返回 `expr1` − `expr2` 表示为从一天到另一天的天数值。`expr1` 和 `expr2` 是日期或日期时间表达式。在计算中只使用值的日期部分。

### date_add

> 原型：date_add(`date`, `interval`, `[unit]`)

向日期添加一个间隔。`date` 参数指定起始日期或日期时间值，`interval` 参数指定要添加到起始日期的间隔值，`unit` 参数指定间隔值的单位。

可用单位有：

- 1: 秒
- 2: 分钟
- 3: 小时
- 4: 天，**默认**
- 5: 周

如果 `date` 或 `interval` 是 NULL，则返回 NULL。

### date_format

> 原型：date_format(`date`, `format`)

根据 `format` 字符串格式化 `date` 值。如果任一参数是 NULL，则函数返回 NULL。

### date_sub

> 原型：date_sub(`date`, `interval`, `[unit]`)

它与 [date_add](#date_add) 相同，但是从 `date` 中减去间隔。

### day

> 原型：day(`date`)

DAY() 是 [DAYOFMONTH()](#dayofmonth) 的同义词。

### dayname

> 原型：dayname(`date`)

返回 `date` 的星期几的名称。名称使用的语言由 `lc_time_names` 系统变量的值控制。如果 `date` 是 NULL，则返回 NULL。

### dayofmonth

> 原型：dayofmonth(`date`)

返回 `date` 的月份中的天数，在 1 到 31 的范围内。如果 `date` 是 NULL，则返回 NULL。

### dayofweek

> 原型：dayofweek(`date`)

返回 `date` 的星期几索引（0 = 星期日，1 = 星期一，... 6 = 星期六）。如果 `date` 是 NULL，则返回 NULL。

### dayofyear

> 原型：dayofyear(`date`)

返回 `date` 的一年中的天数，在 1 到 366 的范围内。如果 `date` 是 NULL，则返回 NULL。

### decode

> 原型：decode(`expr`, `value1`, `result1`, `value2`, `result2`, ..., [`default`])

如果 `expr` 等于 `value1`，返回 `result1`。否则，如果 `expr` 等于 `value2`，返回 `result2`，依此类推。

如果 `expr` 与任何值都不匹配，则返回 `default`。如果省略了 `default`，则返回 NULL。

### degrees

> 原型：degrees(`x`)

返回参数 `x`，从弧度转换为度。如果 `x` 是 NULL，则返回 NULL。

### double

> 原型：double(`x`)

将 `x` 转换为双精度。如果不是有效数字，则返回 0。

### elt

> 原型：elt(`n`, `str1`, `str2`, ...)

返回字符串列表中的第 `n` 个元素：

如果 `n` = 1，则返回 `str1`，如果 `n` = 2，则返回 `str2`，依此类推。如果 `n` 小于 1、大于参数数量或为 NULL，则返回 NULL。

ELT() 是 [FIELD()](#field) 的补充。

### exp

> 原型：exp(`x`)

返回 e（自然对数的底数）的 `x` 次幂。

这个函数的逆函数是 [LOG()](#log)（仅使用单个参数）或 [LN()](#ln)。

如果 `x` 是 NULL，则此函数返回 NULL。

### export

> 原型：export(`table_name`, `file_path`, `[with_title]`, `[with_line_no]`, `[delimiter]`)

将表导出到 CSV 文件。

`table_name` 指定要导出的表的名称。`file_path` 指定要导出到的文件的路径。`with_title` 参数确定文件的第一行是否包含列名，默认为 false。`with_line_no` 参数确定文件的第一列是否包含行号，默认为 false。`delimiter` 参数确定文件中使用的分隔符，默认为 ','。

### field

> 原型：field(`str`, `str1`, `str2`, ...)

返回 `str` 在后续字符串列表中的索引（位置）。如果 `str` 未找到，则返回 0。

FIELD() 是 [ELT()](#elt) 的补充。

### floor

> 原型：floor(`x`)

返回不大于 `x` 的最大整数值。如果 `x` 是 NULL，则返回 NULL。

### from_base64

> 原型：from_base64(`str`)

采用 base-64 编码规则编码的字符串并返回解码结果作为二进制字符串。

如果参数是 NULL 或不是有效的 base-64 字符串，则结果为 NULL。

### from_unixtime

> 原型：from_unixtime(`timestamp`, `[format]`)

将 unix 时间戳表示为字符串，字符串值格式由 `format` 参数给出。如果省略了 `format`，则默认格式为 `%F %T`。

### get

> 原型：get(`key`)

返回名为 `key` 的环境变量的值，如果变量不存在，则返回空字符串。

`@key` 是 `get(key)` 的同义词。

### greatest

> 原型：greatest(`x1`, `x2`, ...)

有两个或更多参数时，返回最大（数值最大的）参数。参数的比较规则与 [LEAST()](#least) 相同。

- 如果任何参数是 NULL，则结果为 NULL。
- 如果任何参数是字符串，则结果为字符串。
- 如果任何参数是实数，则结果为实数。
- 如果所有参数都是整数值，则结果为整数。

如果任何参数是 NULL，则返回 NULL。

### group_concat

> 原型：group_concat(`expr`)

返回一个字符串结果，包含从组中连接的非 NULL 值。如果任何参数是 NULL，则返回 NULL。

### hex

> 原型：hex(`n` 或 `s`)

此函数可用于获取十进制数或字符串的十六进制表示。

### hour

> 原型：hour(`time`)

返回时间的小时。返回值的范围是 0 到 23。如果 `time` 是 NULL，则返回 NULL。

### if

> 原型：if(`condition`, `true_value`, `false_value`)

如果 `condition` 为真，则返回 `true_value`，否则返回 `false_value`。

### ifnull

> 原型：ifnull(`expr1`, `expr2`)

如果 `expr1` 是 NULL，则返回 `expr2`，否则返回 `expr1`。这与两个参数的 [COALESCE()](#coalesce) 函数相同。

### import

> 原型：import(`table_name`, `file_path`, `[with_title]`, `[columns]`, `[delimiter]`)

从文件中导入数据到表中。文件预期为 CSV 格式。

`table_name` 指定要导入数据的表的名称。`file_path` 指定要导入的文件的路径。`with_title` 参数确定文件的第一行是否包含列名，默认为 false。`columns` 参数确定文件的列数，如果省略，则自动确定列数。`delimiter` 参数确定文件中使用的分隔符，默认为 '\\s+'。

### insert

> 原型：insert(`str`, `pos`, `len`, `newstr`)

返回字符串 `str`，在位置 `pos` 开始的子字符串长度为 `len` 的部分被字符串 `newstr` 替换。如果 `pos` 不在字符串的长度内，则返回原始字符串。如果 `len` 不在剩余字符串的长度内，则从位置 `pos` 开始替换字符串的其余部分。

如果任何参数是 NULL，则返回 NULL。

### instr

> 原型：instr(`str`, `substr`)

返回子字符串 `substr` 在字符串 `str` 中第一次出现的位置。这与 [LOCATE()](#locate) 的两参数形式相同，除了参数的顺序相反。

### isnull

> 原型：isnull(`expr`)

如果 `expr` 是 NULL，则返回 `true`，否则返回 `false`。

### int

> 原型：int(`x`)

将 `x` 转换为整数。如果不是有效数字，则返回 0。

### last_day

> 原型：last_day(`date`)

接受一个日期或日期时间值，并返回该月的最后一天对应的值。如果 `date` 无效或为 NULL，则返回 NULL。

### lcase

> 原型：lcase(`str`)

LCASE() 是 [LOWER()](#lower) 的同义词。

### least

> 原型：least(`x1`, `x2`, ...)

有两个或更多参数时，返回最小（数值最小的）参数。参数的比较规则与 [GREATEST()](#greatest) 相同。

### left

> 原型：left(`str`, `len`)

从字符串 `str` 返回最左边的 `len` 个字符，或者如果任何参数是 NULL，则返回 NULL。

### length

> 原型：length(`str`)

返回字符串 str 的长度。如果 str 是 NULL，则返回 NULL。

### ln

> 原型：ln(`x`)

返回 `x` 的自然对数；即以 e 为底的对数。

如果 `x` 小于或等于 0，则返回 NULL。如果 `x` 是 NULL，则返回 NULL。

这个函数是 [LOG()](#log) 单参数形式的同义词。

这个函数的逆函数是 [EXP()](#exp)。

### localtime

> 原型：localtime()

LOCALTIME() 是 [NOW()](#now) 的同义词。

### localtimestamp

> 原型：localtimestamp()

LOCALTIMESTAMP() 是 [NOW()](#now) 的同义词。

### locate

> 原型：locate(`substr`, `str`, `[pos]`)

返回子字符串 `substr` 在字符串 `str` 中第一次出现的起始位置，从位置 `pos` 开始，当省略 `pos` 时，默认从位置 1 开始。

如果 `substr` 不在 `str` 中，则返回 0。如果任何参数是 NULL，则返回 NULL。

### log

> 原型：log(`[b]`, `x`)

如果使用一个参数调用，此函数返回 `x` 的自然对数。如果 `x` 小于或等于 0，则返回 NULL。如果 `x` 或 `b` 是 NULL，则返回 NULL。

这个函数（当使用单个参数时）的逆函数是 [EXP()](#exp)。

如果使用两个参数调用，此函数返回以 `b` 为底的 `x` 的对数。

如果 `x` 小于或等于 0，或者如果 `b` 小于或等于 1，则返回 NULL。

`LOG(b, x)` 等同于 `LOG(x)/LOG(b)`。

### log2

> 原型：log2(`x`)

返回 `x` 的以 2 为底的对数。如果 `x` 小于或等于 0，则返回 NULL。如果 `x` 是 NULL，则返回 NULL。

这个函数等同于表达式 `LOG(x)/LOG(2)`。

### log10

> 原型：log10(`x`)

返回 `x` 的以 10 为底的对数。如果 `x` 小于或等于 0，则返回 NULL。如果 `x` 是 NULL，则返回 NULL。

这个函数等同于 `LOG(10, x)`。

### lower

> 原型：lower(`str`)

返回将字符串 `str` 中的所有字符转换为小写，如果 `str` 是 NULL，则返回 NULL。

### lpad

> 原型：lpad(`str`, `len`, `padstr`)

返回字符串 `str`，用字符串 `padstr` 左填充到 `len` 个字符的长度。如果 `str` 比 `len` 长，则返回值缩短为 `len` 个字符。

如果任何参数是 NULL，则返回 NULL。

### ltrim

> 原型：ltrim(`str`)

返回移除前导空格字符的字符串 `str`。如果 `str` 是 NULL，则返回 NULL。

### makedate

> 原型：makedate(`year`, `dayofyear`)

给定 `year` 和 `dayofyear` 值，返回一个日期。`dayofyear` 必须大于 0，否则结果为 NULL。如果任一参数是 NULL，则结果也为 NULL。

### maketime

> 原型：maketime(`hour`, `minute`, `second`)

根据 `hour`、`minute` 和 `second` 参数计算返回一个时间值。如果任何参数是 NULL，则返回 NULL。

### max

> 原型：max(`expr`)

返回 `expr` 的最大值。MAX() 可以接收字符串参数；在这种情况下，它返回最大字符串值。

### md5

> 原型：md5(`str`)

计算 `str` 的 MD5 128 位校验和。值作为 32 个十六进制数字的字符串返回，如果参数是 NULL，则返回 NULL。

### mid

> 原型：mid(`str`, `pos`, `[len]`)

MID() 是 [SUBSTRING()](#substring) 的同义词。

### min

> 原型：min(`expr`)

返回 `expr` 的最小值。MIN() 可以接收字符串参数；在这种情况下，它返回最小字符串值。

### minute

> 原型：minute(`time`)

返回 `time` 的分钟，在 0 到 59 的范围内，如果 `time` 是 NULL，则返回 NULL。

### mod

> 原型：mod(`n`, `m`)

模运算。返回 `n` 除以 `m` 的余数。如果 `m` 或 `n` 是 NULL，则返回 NULL。

`MOD(n, 0)` 返回 NULL。

### month

> 原型：month(`date`)

返回 `date` 的月份，在 1 到 12 的范围内，对应一月到十二月。如果 `date` 是 NULL，则返回 NULL。

### monthname

> 原型：monthname(`date`)

返回 `date` 的月份的全名。名称使用的语言由 `lc_time_names` 系统变量的值控制。如果 `date` 是 NULL，则返回 NULL。

### now

> 原型：now()

以 'YYYY-mm-dd HH:MM:SS' 格式返回当前日期和时间。

### nullif

> 原型：nullif(`expr1`, `expr2`)

如果 `expr1` 等于 `expr2`，则返回 NULL；否则返回 `expr1`。这与 `IF(expr1 = expr2, NULL, expr1)` 相同。

### oct

> 原型：oct(`n`)

返回 `n` 的八进制值的字符串表示。这等同于 `CONV(N, 10, 8)`。如果 `n` 是 NULL，则返回 NULL。

### pi

> 原型：pi()

返回 π（圆周率）的值。

### position

> 原型：position(`substr`, `str`)

这与 [LOCATE()](#locate) 的两参数形式相同。

### pow

> 原型：pow(`x`, `y`)

返回 `x` 的 `y` 次幂的值。如果 `x` 或 `y` 是 NULL，则返回 NULL。

### power

> 原型：power(`x`, `y`)

这是 [POW()](#pow) 的同义词。

### quarter

> 原型：quarter(`date`)

返回 `date` 的一年中的季度，在 1 到 4 的范围内，或者如果 `date` 是 NULL，则返回 NULL。

### radians

> 原型：radians(`x`)

返回参数 `x`，从度转换为弧度。（注意 π 弧度等于 180 度。）

如果 `x` 是 NULL，则返回 NULL。

### rand

> 原型：rand(`[n]`)

返回一个在 0 <= `v` < 1.0 范围内的随机浮点数值 `v`。

如果指定了整数参数 `n`，则将其用作种子值。对于相等的参数值，`RAND`(`n`) 每次都返回相同的值。

### repeat

> 原型：repeat(`str`, `count`)

返回由字符串 `str` 重复 `count` 次组成的字符串。如果 `count` 小于 1，则返回空字符串。如果 `str` 或 `count` 是 NULL，则返回 NULL。

### replace

> 原型：replace(`str`, `from`, `to`)

返回字符串 `str`，将所有出现的字符串 `from` 替换为字符串 `to`。这个函数是不区分大小写的。

如果任何参数是 NULL，则返回 NULL。

### reverse

> 原型：reverse(`str`)

返回字符顺序被反转的字符串 `str`，如果 `str` 是 NULL，则返回 NULL。

### right

> 原型：right(`str`, `len`)

从字符串 `str` 返回最右边的 `len` 个字符，如果任何参数是 NULL，则返回 NULL。

### round

> 原型：round(`x`, `[d]`)

将参数 `x` 四舍五入到 `d` 小数位。`d` 默认为 0，如果没有指定。`d` 可以是负数，以使 `x` 的值左边的 `d` 位数字变为零。

如果 `x` 或 `d` 是 NULL，则返回 NULL。

### rpad

> 原型：rpad(`str`, `len`, `padstr`)

返回字符串 `str`，用字符串 `padstr` 右填充到 `len` 个字符的长度。如果 `str` 比 `len` 长，则返回值缩短为 `len` 个字符。

如果任何参数是 NULL，则返回 NULL。

### rtrim

> 原型：rtrim(`str`)

返回移除尾随空格字符的字符串 `str`。如果 `str` 是 NULL，则返回 NULL。

### second

> 原型：second(`time`)

返回 `time` 的秒，在 0 到 59 的范围内，如果 `time` 是 NULL，则返回 NULL。

### sec_to_time

> 原型：sec_to_time(`seconds`)

将 `seconds` 参数转换为小时、分钟和秒。如果 `seconds` 是 NULL，则返回 NULL。

### serial

> 原型：serial([`n`])

返回一个随机序列号。如果指定了整数参数 `n`，则将其用作序列号的长度，否则，长度为 8。

### set

> 原型：set(`key`, `value`)

设置或更新（如果存在）环境变量 `key` 为 `value`。

### sha

> 原型：sha(`str`)

SHA() 与 [SHA1()](#sha1) 同义。

### sha1

> 原型：sha1(`str`)

计算字符串的 SHA-1 160 位校验和。值作为 40 个十六进制数字的字符串返回，如果参数是 NULL，则返回 NULL。

### sha2

> 原型：sha2(`str`, `[n]`)

计算 SHA-2 家族的哈希函数（SHA-224、SHA-256、SHA-384 和 SHA-512）。第一个参数是要哈希的明文字符串。第二个参数指示结果期望的位数，必须为 224、256、384、512 或 0（等同于 256）。如果省略第二个参数，则默认为 256。

如果任一参数是 NULL 或哈希长度不是允许的值之一，则返回值为 NULL。否则，函数结果为包含期望位数的哈希值。

### sha224

> 原型：sha224(`str`)

这个函数与 [SHA2()](#sha2) 相同，哈希长度为 224 位。

### sha256

> 原型：sha256(`str`)

这个函数与 [SHA2()](#sha2) 相同，哈希长度为 256 位。

### sha384

> 原型：sha384(`str`)

这个函数与 [SHA2()](#sha2) 相同，哈希长度为 384 位。

### sha512

> 原型：sha512(`str`)

这个函数与 [SHA2()](#sha2) 相同，哈希长度为 512 位。

### sign

> 原型：sign(`x`)

根据参数 `x` 是负数、零还是正数，返回 -1、0 或 1。如果 `x` 是 NULL，则返回 NULL。

### sin

> 原型：sin(`x`)

返回 `x` 的正弦值，其中 `x` 以弧度给出。如果 `x` 是 NULL，则返回 NULL。

### sleep

> 原型：sleep(`milliseconds`)

暂停指定的毫秒数。返回 0。

注意，由于程序是单线程的，如果有多行，sleep 函数会多次执行。

### space

> 原型：space(`n`)

返回由 `n` 个空格字符组成的字符串，如果 `n` 是 NULL，则返回 NULL。

### sqrt

> 原型：sqrt(`x`)

返回非负数 `x` 的平方根。如果 `x` 是 NULL，则函数返回 NULL。

### strcmp

> 原型：strcmp(`str1`, `str2`)

如果字符串相同，返回 0，如果第一个参数小于第二个，返回 -1，否则返回 1。

如果任一参数是 NULL，则返回 NULL。

### string

> 原型：string(`x`)

将 `x` 转换为字符串。

### str_to_date

> 原型：str_to_date(`str`, `format`)

将参数解析为日期。如果 `str` 或 `format` 是 NULL，则函数返回 NULL。这个函数与 [DATE_FORMAT()](#date_format) 相反。

### subdate

> 原型：subdate(`date`, `days`)

SUBDATE() 是 [DATE_SUB()](#date_sub) 两参数形式的同义词。

### substr

> 原型：substr(`str`, `pos`, `[len]`)

SUBSTR() 是 [SUBSTRING()](#substring) 的同义词。

### substring

> 原型：substring(`str`, `pos`, `[len]`)

从字符串 `str` 返回长度为 `len` 的子字符串，从位置 `pos` 开始。字符串的第一个字符位于位置 1。也可以对 `pos` 使用负值。在这种情况下，子字符串的开始是字符串末尾距 `pos` 个字符的位置，而不是开头。`pos` 的值为 0 时返回空字符串。

如果任何参数是 NULL，则返回 NULL。如果 `len` 小于 1，则结果为空字符串。

### substring_index

> 原型：substring_index(`str`, `delim`, `count`)

返回字符串 `str` 中 `delim` 分隔符出现 `count` 次之前的子字符串。如果 `count` 是正数，返回最左边的分隔符（从左边数）左侧的所有内容。如果 `count` 是负数，返回最右边的分隔符（从右边数）右侧的所有内容。

如果任何参数是 NULL，则返回 NULL。

### subtime

> 原型：subtime(`date`, `expr`)

与 [ADDTIME()](#addtime) 相同，但是减去。

### sum

> 原型：sum(`expr`)

返回 `expr` 的总和（跳过 NULL 值）。如果所有的 `expr` 都是 NULL，则返回 NULL。

### sysdate

> 原型：sysdate()

它是 [NOW()](#now) 的同义词。

### tan

> 原型：tan(`x`)

返回 `x` 的正切值，其中 `x` 以弧度给出。如果 `x` 是 NULL，则返回 NULL。

### time

> 原型：time(`expr`)

提取时间或日期时间表达式 `expr` 的时间部分，并将其作为字符串返回。如果 `expr` 是 NULL，则返回 NULL。

### timediff

> 原型：timediff(`time1`, `time2`)

返回 `time1` − `time2` 表示为时间值。如果任一参数是 NULL，则返回 NULL。

### time_to_sec

> 原型：time_to_sec(`time`)

将 `time` 参数转换为秒。如果 `time` 是 NULL，则返回 NULL。

### to_base64

> 原型：to_base64(`str`)

将字符串参数转换为 base-64 编码形式，并返回结果作为字符字符串，与连接的字符集和校对有关。如果参数不是字符串，它在转换之前被转换为字符串。

如果参数是 NULL，则结果为 NULL。

Base-64 编码的字符串可以使用 [FROM_BASE64()](#from_base64) 函数进行解码。

### trim

> 原型：trim(`str`)

返回移除所有前导和尾随空格字符的字符串 `str`。如果 `str` 是 NULL，则返回 NULL。

### truncate

> 原型：truncate(`x`, `[d]`)

返回数字 `x`，截断到 `d` 小数位。如果 `d` 是 0，则结果没有小数点或小数部分。`d` 可以是负数，以使 `x` 的值左边的 `d` 位数字变为零。

如果 `x` 或 `d` 是 NULL，则返回 NULL。

### ucase

> 原型：ucase(`str`)

UCASE() 是 [UPPER()](#upper) 的同义词。

### unhex

> 原型：unhex(`hex_string`)

返回包含十六进制参数字符表示的字符串。

参数字符串中的字符必须是合法的十六进制数字。如果参数包含任何非十六进制数字，或者参数本身是 NULL，则结果为 NULL。

它是 [HEX()](#hex) 函数的反函数。

### unix_timestamp

> 原型：unix_timestamp(`[date]`)

返回参数的值，作为自 '1970-01-01 00:00:00' UTC 以来的秒数。如果省略 `date`，则使用当前日期和时间。

### unset

> 原型：unset(`key`)

删除名为 `key` 的环境变量。

### upper

> 原型：upper(`str`)

返回将字符串 `str` 中的所有字符转换为大写，如果 `str` 是 NULL，则返回 NULL。

### uuid

> 原型：uuid()

返回符合 RFC 4122 描述的 UUID 第 4 版的字符串。

### version

> 原型：version()

返回应用程序的版本。

### week

> 原型：week(`date`, `[first_day]`, `[mode]`)

此函数返回 `date` 的周数。`first_day` 参数确定一周的第一天，`mode` 参数确定周数的计算方式。`first_day` 和 `mode` 的默认值分别为 1 和 3。

`first_day` 可以是 0（星期日）或 1（星期一），... 6（星期六）。`mode` 可以是以下值之一：

- 0：在 0-53 范围内，周 0 是包含 `first_day` 的这一年的第一周。
- 1：在 0-53 范围内，周 0 是包含 4 天以上在这一年的周。
- 2：在 1-53 范围内，周 1 是包含 `first_day` 的这一年的第一周。
- 3：在 1-53 范围内，周 1 是包含 4 天以上在这一年的第一周。

### weekday

> 原型：weekday(`date`)

返回 `date` 的星期几索引（0 = 星期日，1 = 星期一，... 6 = 星期六）。如果 `date` 是 NULL，则返回 NULL。

### weekofyear

> 原型：weekofyear(`date`)

以数字形式返回 `date` 的日历周，范围从 1 到 53。如果 `date` 是 NULL，则返回 NULL。

这个函数等同于 `WEEK(date, 1, 3)`。

### year

> 原型：year(`date`)

返回 `date` 的年份。年份作为 1000 到 9999 之间的数字返回。如果 `date` 是 NULL，则返回 NULL。

### yearweek

> 原型：yearweek(`date`, `[first_day]`, `[mode]`)

返回 `date` 的年份和周数。结果中的年份可能与 `date` 参数中的年份不同，对于年初和年末的第一周和最后一周。如果 `date` 是 NULL，则返回 NULL。

`first_day` 和 `mode` 参数的工作方式与 [WEEK()](#week) 完全相同。
