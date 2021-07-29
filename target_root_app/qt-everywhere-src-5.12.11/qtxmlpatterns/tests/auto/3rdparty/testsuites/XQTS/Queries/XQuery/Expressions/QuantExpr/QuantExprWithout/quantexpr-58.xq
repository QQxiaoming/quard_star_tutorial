(: Name: quantExpr-58 :)
(: Description: Simple quantified expression using "every" keyword, use of multiple variable and the xs:boolean type  :)

every $x in ("true", "false"), $y in ("false","true") satisfies xs:boolean($x) = xs:boolean($y)