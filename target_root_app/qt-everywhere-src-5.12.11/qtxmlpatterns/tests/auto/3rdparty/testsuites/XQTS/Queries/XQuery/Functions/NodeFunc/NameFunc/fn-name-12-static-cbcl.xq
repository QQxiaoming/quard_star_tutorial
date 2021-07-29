(: Name: fn-name-12:)
(: Description: Evaluation of the fn:name function with argument that uses the "descendant-or-self" axes:)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]) 
return fn:name(($h/descendant-or-self::empnum)[1])
