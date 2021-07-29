(: Name: quantExpr-59 :)
(: Description: Simple quantified expression using "every" keyword, use of multiple variable and the xs:dateTime type  :)

every $x in ("1980-05-05T13:13:13Z", "1980-05-05T13:13:13Z"), $y in ("1980-05-05T13:13:13Z","1980-05-05T13:13:13Z") satisfies xs:dateTime($x) = xs:dateTime($y)