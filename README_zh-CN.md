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

> 查询语句必须用双引号括起来。

## 命令行选项

- `-h`, `--help`：显示此帮助信息并退出。
- `-v`, `--version`：输出版本信息并退出。
- `-t`, `--title`：输出标题。
- `-l`, `--line-no`：输出行号。
- `-f`, `--file=FILE`：从文件中读取数据。
- `-d`, `--delimiter=DELIMITER`：指定字符作为字段分隔符。
- `-c`, `--columns=COLUMNS`：指定数据源的列数。

## 查询语句

查询语句是类SQL的select句型：

```
[WITH] select {columns} [FROM] [WHERE] [GROUP BY] [ORDER BY] [LIMITS]
```

> 关键字和函数不区分大小写。

语法与标准`SQL`一致，但：

- 输入数据被放置在一个名为“std”的表中。
- 不支持子查询，请改用`with`。
- Group by只允许字段名或字段索引（在select子句中）。

## 示例

```bash
ps -aux | sql -tlc11 "select col1 as user, col2 as pid, col9 as start, col11 as command \
from std where col2 not like 'PID';"
```

## 支持的函数

> 🟡 计划中
>
> 🟢 已实现
