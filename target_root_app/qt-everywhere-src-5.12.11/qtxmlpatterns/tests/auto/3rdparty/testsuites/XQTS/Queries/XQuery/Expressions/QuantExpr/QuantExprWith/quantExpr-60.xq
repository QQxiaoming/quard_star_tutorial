(: Name: quantExpr-60 :)
(: Description: Simple quantified expression using "every" keyword, use of multiple variable and the xs:date type  :)

every $x in ("1985-07-05Z", "1985-07-05Z"), $y in ("1985-07-05Z","1985-07-05Z") satisfies xs:date($x) = xs:date($y)