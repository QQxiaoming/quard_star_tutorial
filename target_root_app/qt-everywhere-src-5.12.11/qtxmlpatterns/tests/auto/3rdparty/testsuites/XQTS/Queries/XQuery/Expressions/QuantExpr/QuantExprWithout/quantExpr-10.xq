(: Name: quantExpr-10 :)
(: Description: Simple quantified expression using "some" keyword and usage of string-length function expression. :)

some $x in (1, 2) satisfies fn:string-length(xs:string($x)) = 1