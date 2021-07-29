(: Name: quantExpr-23 :)
(: Description: Simple quantified expression using "some" keyword with multiple variables, and string function. :)

some $x in (1,2,3), $y in (4,5,6) satisfies fn:string($x) = fn:string($y)