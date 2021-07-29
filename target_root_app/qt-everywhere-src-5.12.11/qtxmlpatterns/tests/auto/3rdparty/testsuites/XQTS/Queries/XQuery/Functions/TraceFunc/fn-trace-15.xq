(: Name: fn-trace-15 :)
(: Description: Simple call of "fn:trace" function that uses another fn-trace as argument. :)

 fn:trace((fn:trace((2+2),"The value of '2 + 2' is:" )) ,"The Value of 'fn:trace(2+2)' is: ")