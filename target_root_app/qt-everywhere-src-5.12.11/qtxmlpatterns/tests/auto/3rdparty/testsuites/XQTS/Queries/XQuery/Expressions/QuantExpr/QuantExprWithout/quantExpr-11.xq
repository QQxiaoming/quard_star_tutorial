(: Name: quantExpr-11 :)
(: Description: Simple quantified expression using "some" keyword and usage of "fn:count" function expression. :)

some $x in (1, 2) satisfies fn:count(($x)) = 1