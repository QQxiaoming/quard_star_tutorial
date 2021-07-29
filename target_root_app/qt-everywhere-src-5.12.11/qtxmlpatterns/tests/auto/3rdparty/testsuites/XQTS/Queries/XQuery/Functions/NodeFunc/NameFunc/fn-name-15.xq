(: Name: fn-name-15:)
(: Description: Evaluation of the fn:name function with argument that uses the "self" axes.  Returns a string:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]) 
return fn:name($h/self::employee)
