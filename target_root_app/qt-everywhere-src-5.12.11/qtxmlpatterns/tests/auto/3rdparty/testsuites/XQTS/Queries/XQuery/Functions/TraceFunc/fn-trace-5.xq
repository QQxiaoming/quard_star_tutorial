(: Name: fn-trace-5 :)
(: Description: Simple call of "fn:trace" function used with a multiplication operation. :)

for $var in (2,3,4,5)
return
 fn:trace($var * 2 ,"The Value of $var * 2 is: ")