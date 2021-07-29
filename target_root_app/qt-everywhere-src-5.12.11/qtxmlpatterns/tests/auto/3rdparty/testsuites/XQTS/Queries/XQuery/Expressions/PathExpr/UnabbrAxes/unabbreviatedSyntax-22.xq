(: Name: unabbreviatedSyntax-22 :)
(: Description: Evaluate "child::employee[fn:position() = fn:last()-1]" Selects the last but one "employee" child of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee[fn:position() = fn:last()-1]