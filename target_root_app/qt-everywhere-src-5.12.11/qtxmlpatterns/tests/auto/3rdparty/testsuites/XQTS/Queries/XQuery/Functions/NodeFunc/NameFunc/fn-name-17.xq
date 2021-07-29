(: Name: fn-name-17:)
(: Description: Evaluation of the fn:name function with argument that uses the "parent::node()". :)
(: The context node is an attribute node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]/@name) 
return fn:name($h/parent::node())
