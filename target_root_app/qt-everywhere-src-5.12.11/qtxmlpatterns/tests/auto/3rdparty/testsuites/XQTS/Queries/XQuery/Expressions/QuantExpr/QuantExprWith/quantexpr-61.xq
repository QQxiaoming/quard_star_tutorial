(: Name: quantExpr-61 :)
(: Description: Simple quantified expression using "some" keyword that binds the declared variables to an xs:integer type. :)

some $x as xs:integer in (1, 2, 3) , $y as xs:integer in (2, 3, 4) 
     satisfies $x + $y = 4