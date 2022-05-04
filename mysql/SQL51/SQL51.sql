/*
SQL51 查找字符串中逗号出现的次数
描述
现有strings表如下：
id指序列号；
string列中存放的是字符串，且字符串中仅包含数字、字母和逗号类型的字符。
id	string
1   10,A,B,C,D
2   A,B,C,D,E,F
3   A,11,B,C,D,E,G

请你统计每个字符串中逗号出现的次数cnt。
以上例子的输出结果如下：
id	cnt
1   4
2   5
3   6

示例1
输入：
drop table if exists strings;
CREATE TABLE strings(
   id int(5)  NOT NULL PRIMARY KEY,
   string  varchar(45) NOT NULL
 );
insert into strings values
(1, '10,A,B'),
(2, 'A,B,C,D'),
(3, 'A,11,B,C,D,E');
复制
输出：
1|2
2|3
3|5
 */

#sql语句
#思路：利用length来计算总共的长度，在计算删除,号之后的长度，即是,号的个数
select id, length(string) - length(replace(string,',','')) as cnt
from strings;
