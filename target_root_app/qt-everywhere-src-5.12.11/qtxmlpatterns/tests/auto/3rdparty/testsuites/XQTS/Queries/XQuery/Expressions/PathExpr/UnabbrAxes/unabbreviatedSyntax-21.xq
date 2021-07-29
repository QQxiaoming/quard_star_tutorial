(: Name: unabbreviatedSyntax-21 :)
(: Description: Evaluate "child::employee[fn:position() = fn:last()]" selects the previous to the one "employee" child of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee[fn:position() = fn:last()]