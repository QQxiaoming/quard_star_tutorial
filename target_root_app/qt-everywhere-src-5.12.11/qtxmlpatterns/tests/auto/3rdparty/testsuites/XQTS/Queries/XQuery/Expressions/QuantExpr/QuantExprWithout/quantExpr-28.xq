(: Name: quantExpr-28 :)
(: Description: Simple quantified expression using "some" keyword, use of multiple variable and the xs:boolean type  :)

some $x in ("true", "false"), $y in ("false","true") satisfies xs:boolean($x) = xs:boolean($y)