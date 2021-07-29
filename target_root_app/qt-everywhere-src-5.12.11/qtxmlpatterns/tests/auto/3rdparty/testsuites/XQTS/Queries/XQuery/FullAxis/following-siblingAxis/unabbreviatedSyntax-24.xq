(: Name: unabbreviatedSyntax-24 :)
(: Description: Evaluate "following-sibling::employee[fn:position() = 1]".  Selects the next employee sibling of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[2]) 
 return $h/following-sibling::employee[fn:position() = 1]