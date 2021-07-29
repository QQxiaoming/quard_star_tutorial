(: Name: quantExpr-25 :)
(: Description: Simple quantified expression using "some" keyword, use of multiple variable and the xs:decimal type  :)

some $x in (1,2,3), $y in (4,5,6) satisfies xs:decimal($x) = xs:decimal($y)