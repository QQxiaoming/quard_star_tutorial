(: Name: quantExpr-53 :)
(: Description: Simple quantified expression using "every" keyword with multiple variables, and string function. :)

every $x in (1,2,3), $y in (4,5,6) satisfies fn:string($x) = fn:string($y)