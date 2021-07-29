(: Name: fn-trace-3 :)
(: Description: Simple call of "fn:trace" function used with an addition operation. :)

for $var in (1,2,3,4,5)
return
 fn:trace($var + 1,"The Value of $var + 1 is: ")