(: Name: fn-trace-9 :)
(: Description: Simple call of "fn:trace" function used with two variables, where the second one uses the first one in a more complex math expression. :)

for $var1 in (2,4,6,8)
let $var2 := (3 + $var1) - (4 * $var1)
return
 fn:trace($var1 + $var2 ,"The Value of $var 1 + $var2 is: ")