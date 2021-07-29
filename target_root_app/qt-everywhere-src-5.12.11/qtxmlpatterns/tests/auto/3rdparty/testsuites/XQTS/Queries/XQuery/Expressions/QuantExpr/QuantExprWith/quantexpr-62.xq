(: Name: quantExpr-62 :)
(: Description: Simple quantified expression using "some" keyword that binds the declared variable to an xs:string type. :)

some $x as xs:string in ("cat","dog","rat")  
     satisfies fn:string-length($x) = 3