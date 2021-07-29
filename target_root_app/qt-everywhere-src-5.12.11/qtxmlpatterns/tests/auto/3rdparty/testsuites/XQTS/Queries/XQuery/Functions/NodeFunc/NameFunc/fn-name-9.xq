(: Name: fn-name-9:)
(: Description: Evaluation of the fn:name function used as an argument to the fn:lower-case function:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]) 
return fn:lower-case(fn:name($h))