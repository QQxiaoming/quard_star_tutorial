(: Name: quantExpr-41 :)
(: Description: Simple quantified expression using "every" keyword and usage of "fn:count" function expression. :)

every $x in (1, 2) satisfies fn:count(($x)) = 1