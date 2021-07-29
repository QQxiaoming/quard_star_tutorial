(: Name: fn-trace-8 :)
(: Description: Simple call of "fn:trace" function used with a modulus operation. :)

for $var in (2,4,6,8)
return
 fn:trace($var mod 2 ,"The Value of $var mod 2 is: ")