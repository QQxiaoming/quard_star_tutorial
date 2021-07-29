(: Name: quantExpr-64 :)
(: Description: Simple quantified expression using "every" keyword that binds the declared variable to an xs:string and xs:integer type respectively. :)

every $x as xs:string in ("cat","dog","rat"), $y as xs:integer in (3, 3, 3)
     satisfies fn:string-length($x) = $y