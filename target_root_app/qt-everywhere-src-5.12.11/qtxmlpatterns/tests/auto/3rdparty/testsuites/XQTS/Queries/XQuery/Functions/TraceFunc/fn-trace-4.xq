(: Name: fn-trace-4 :)
(: Description: Simple call of "fn:trace" function used with a subtraction operation. :)

for $var in (2,3,4,5)
return
 fn:trace($var - 1,"The Value of $var - 1 is: ")