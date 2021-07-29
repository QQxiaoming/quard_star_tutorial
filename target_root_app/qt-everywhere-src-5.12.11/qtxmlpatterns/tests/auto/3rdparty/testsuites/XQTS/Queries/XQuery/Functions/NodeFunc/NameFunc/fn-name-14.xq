(: Name: fn-name-14:)
(: Description: Evaluation of the fn:name function used as argument to the fn:concat function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]) 
return fn:concat(fn:name($h),"A String")
