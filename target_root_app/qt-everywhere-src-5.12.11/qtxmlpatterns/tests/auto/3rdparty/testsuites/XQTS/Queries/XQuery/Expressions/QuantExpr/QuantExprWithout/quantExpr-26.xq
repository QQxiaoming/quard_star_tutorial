(: Name: quantExpr-26 :)
(: Description: Simple quantified expression using "some" keyword, use of multiple variable and the xs:float type  :)

some $x in (1,2,3), $y in (4,5,6) satisfies xs:float($x) = xs:float($y)