(: Name: fn-name-10:)
(: Description: Evaluation of the fn:name function with argument that uses the "parent" axes:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]) 
return fn:name($h/parent::node())
