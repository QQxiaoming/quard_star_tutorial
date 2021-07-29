(: Name: quantExpr-65 :)
(: Description: Simple quantified expression using "some" keyword that binds the declared variable to an xs:integer and xs:float type respectively. :)

some $x as xs:integer in (1, 2, 3), $y as xs:float in (xs:float(2), xs:float(3))
     satisfies $x + $y = 5