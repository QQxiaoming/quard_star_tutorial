(: Name: fn-trace-2 :)
(: Description: Simple call of "fn:trace" function with integer value. :)

let $var := xs:integer("123")
return
 fn:trace($var,"The Value of $var is: ")
