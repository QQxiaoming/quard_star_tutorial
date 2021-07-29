(: Name: unabbreviatedSyntax-20 :)
(: Description: Evaluate "child::employee[fn:position() = 1]". Selects the first employee child of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee[fn:position() = 1]