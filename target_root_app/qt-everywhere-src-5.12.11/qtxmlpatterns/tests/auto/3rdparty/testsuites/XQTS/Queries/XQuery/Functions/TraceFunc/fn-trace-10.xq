(: Name: fn-trace-10 :)
(: Description: Simple call of "fn:trace" function used with string manipulation (fn:concat). :)

for $var in ("aa","bb","cc","dd","ee")
return
 fn:trace(fn:concat($var,$var) ,"The Value of concat($var,$var) is: ")