(: Name: fn-trace-11 :)
(: Description: Simple call of "fn:trace" function used numbers manipulation queried from an xml file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works//hours)
return
 fn:trace(($var div 2) ,"The Value of hours div/2 is: ")