(: Name: quantExpr-24 :)
(: Description: Simple quantified expression using "some" keyword, use of multiple variable and the xs:integer type  :)

some $x in (1,2,3), $y in (4,5,6) satisfies xs:integer($x) = xs:integer($y)