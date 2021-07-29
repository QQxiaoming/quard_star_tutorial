(: Name: fn-trace-7 :)
(: Description: Simple call of "fn:trace" function used with a division (idiv operator) operation. :)

for $var in (2,4,6,8)
return
 fn:trace($var idiv 2 ,"The Value of $var idiv 2 is: ")