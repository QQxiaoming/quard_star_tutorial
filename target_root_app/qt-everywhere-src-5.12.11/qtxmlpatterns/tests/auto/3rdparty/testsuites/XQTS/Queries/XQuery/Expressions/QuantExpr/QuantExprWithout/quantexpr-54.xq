(: Name: quantExpr-54 :)
(: Description: Simple quantified expression using "every" keyword, use of multiple variable and the xs:integer type  :)

every $x in (1,2,3), $y in (4,5,6) satisfies xs:integer($x) = xs:integer($y)