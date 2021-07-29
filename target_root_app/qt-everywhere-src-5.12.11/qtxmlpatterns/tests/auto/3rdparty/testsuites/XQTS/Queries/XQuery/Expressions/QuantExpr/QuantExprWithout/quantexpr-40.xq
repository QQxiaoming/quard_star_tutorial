(: Name: quantExpr-40 :)
(: Description: Simple quantified expression using "every" keyword and usage of string-length function expression. :)

every $x in (1, 2) satisfies fn:string-length(xs:string($x)) = 1