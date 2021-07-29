(: Name: unabbreviatedSyntax-25 :)
(: Description: Evaluate "preceding-sibling::employee[fn:position() = 1]".  Selects the previous employee sibling of the context node. :)			  

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[2]) 
 return $h/preceding-sibling::employee[fn:position() = 1]