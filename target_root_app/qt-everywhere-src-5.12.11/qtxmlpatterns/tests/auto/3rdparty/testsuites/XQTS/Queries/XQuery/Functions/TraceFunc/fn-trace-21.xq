(: Name: fn-trace-21 :)
(: Description: Simple call of "fn:trace" function involving string manipulations with upper/lower case functions. :)

 fn:trace((for $var in (fn:upper-case("a"),fn:lower-case("B")) return (fn:lower-case($var),fn:upper-case($var))) ,"The value of the complex expression on the other argument is:")