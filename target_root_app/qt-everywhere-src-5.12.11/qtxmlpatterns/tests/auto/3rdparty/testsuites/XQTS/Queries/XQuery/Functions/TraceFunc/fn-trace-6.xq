(: Name: fn-trace-6 :)
(: Description: Simple call of "fn:trace" function used with a division (div operator) operation. :)

for $var in (2,4,6,8)
return
 fn:trace($var div 2 ,"The Value of $var div 2 is: ")