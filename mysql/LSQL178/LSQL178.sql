/*
178. 分数排名
表: Scores
+-------------+---------+
| Column Name | Type    |
+-------------+---------+
| id          | int     |
| score       | decimal |
+-------------+---------+
Id是该表的主键。
该表的每一行都包含了一场比赛的分数。Score是一个有两位小数点的浮点值。
编写 SQL 查询对分数进行排序。排名按以下规则计算:
分数应按从高到低排列。
如果两个分数相等，那么两个分数的排名应该相同。
在排名相同的分数后，排名数应该是下一个连续的整数。换句话说，排名之间不应该有空缺的数字。
按 score 降序返回结果表。
查询结果格式如下所示。
示例 1:

输入: 
Scores 表:
+----+-------+
| id | score |
+----+-------+
| 1  | 3.50  |
| 2  | 3.65  |
| 3  | 4.00  |
| 4  | 3.85  |
| 5  | 4.00  |
| 6  | 3.65  |
+----+-------+
输出: 
+-------+------+
| score | rank |
+-------+------+
| 4.00  | 1    |
| 4.00  | 1    |
| 3.85  | 2    |
| 3.65  | 3    |
| 3.65  | 3    |
| 3.50  | 4    |
+-------+------+

*/


/*思路：分为两部分来查，
 第一部分来查询降序排列的分数，第二部分查询每个分数对应的排名
 第一部分不难写
 select a.score as score
 from Scores as a
 order by score desc;

 而第二部分，假设给你一个分数X，你怎么求出它的排名?
 我们可以取出大于等于X的所有分数集合H，将H去重后的元素个数就是X的排名
 比如你考了99分，但最高的就只有99分，那么去重之后集合H里就只有99一个元素，个数为1，因此你的Rank为1。
 先提取集合H：
 select b.socre from Scores as b where b.score>= X 
 我们要的是集合H去重之后的元素个数，因此升级为：
 (select count(distinct(b.score) from Scores as b where b.score >= X)) as `rank`

 而从结果的角度来看，第二部分的Rank是对应第一部分的分数来的，所以这里的X就是上面的a.Score，把两部分结合在一起为：
 select a.score as score, (select count(distinct b.score) from Scores as b where b.score >= a.score) as `rank`
 from Scores as a
 order by score desc;
 */
