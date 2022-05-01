/*
SQL1 查找最晚入职员工的所有信息
描述
有一个员工employees表简况如下:
emp_no	birth_date	first_name	last_name	gender	hire_date
10001	  1953-09-02	Georgi      Facello    M      1986-06-26 
10002   1964-06-02	Bezalel     Simmel     F      1985-11-21
10003   1959-12-03	Parto       Bamford    M      1986-08-28
10004   1954-05-01	Christian   Koblick     M     1986-12-01 


请你查找employees里最晚入职员工的所有信息，以上例子输出如下:
emp_no	birth_date	first_name	last_name	gender	hire_date
10004   1954-05-01	Christian   Koblick     M     1986-12-01 

示例1
输入：
drop table if exists  `employees` ; 
CREATE TABLE `employees` (
`emp_no` int(11) NOT NULL,
`birth_date` date NOT NULL,
`first_name` varchar(14) NOT NULL,
`last_name` varchar(16) NOT NULL,
`gender` char(1) NOT NULL,
`hire_date` date NOT NULL,
PRIMARY KEY (`emp_no`));
INSERT INTO employees VALUES(10001,'1953-09-02','Georgi','Facello','M','1986-06-26');
INSERT INTO employees VALUES(10002,'1964-06-02','Bezalel','Simmel','F','1985-11-21');
INSERT INTO employees VALUES(10003,'1959-12-03','Parto','Bamford','M','1986-08-28');
INSERT INTO employees VALUES(10004,'1954-05-01','Chirstian','Koblick','M','1986-12-01');
INSERT INTO employees VALUES(10005,'1955-01-21','Kyoichi','Maliniak','M','1989-09-12');
INSERT INTO employees VALUES(10006,'1953-04-20','Anneke','Preusig','F','1989-06-02');
INSERT INTO employees VALUES(10007,'1957-05-23','Tzvetan','Zielinski','F','1989-02-10');
INSERT INTO employees VALUES(10008,'1958-02-19','Saniya','Kalloufi','M','1994-09-15');
INSERT INTO employees VALUES(10009,'1952-04-19','Sumant','Peac','F','1985-02-18');
INSERT INTO employees VALUES(10010,'1963-06-01','Duangkaew','Piveteau','F','1989-08-24');
INSERT INTO employees VALUES(10011,'1953-11-07','Mary','Sluis','F','1990-01-22');
复制
输出：
10008|1958-02-19|Saniya|Kalloufi|M|1994-09-15
 
解题sql：
select * 
from employees
order by hire_date desc
limit 1;
*/
