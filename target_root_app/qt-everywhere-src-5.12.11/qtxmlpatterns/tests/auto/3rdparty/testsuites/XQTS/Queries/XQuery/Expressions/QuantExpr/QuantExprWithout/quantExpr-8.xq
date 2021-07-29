(: Name: quantExpr-8 :)
(: Description: Simple quantified expression using "some" keyword and usage of avg function expression. :)

some $x in (1, 2) satisfies fn:avg(($x, 1)) = 1