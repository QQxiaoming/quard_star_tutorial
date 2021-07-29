(: Name: quantExpr-55 :)
(: Description: Simple quantified expression using "every" keyword, use of multiple variable and the xs:decimal type  :)

every $x in (1,2,3), $y in (4,5,6) satisfies xs:decimal($x) = xs:decimal($y)