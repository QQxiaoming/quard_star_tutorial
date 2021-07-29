(: Name: quantExpr-63 :)
(: Description: Simple quantified expression using "every" keyword that binds the declared variable to an xs:string type. :)

every $x as xs:string in ("cat","dog","rat")  
     satisfies fn:string-length($x) = 3