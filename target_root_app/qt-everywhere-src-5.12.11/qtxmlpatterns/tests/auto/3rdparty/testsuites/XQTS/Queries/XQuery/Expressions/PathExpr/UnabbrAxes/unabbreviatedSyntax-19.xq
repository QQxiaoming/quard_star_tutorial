(: Name: unabbreviatedSyntax-19 :)
(: Description: Evaluate "/descendant::employee/child::pnum" - Selects all the pnum elements that have an "employee" parent and that are in the same document as the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/descendant::employee/child::pnum