(: Name: unabbreviatedSyntax-15 :)
(: Description: Evaluate more on child/descendant (child::employee/descendant:empnum)- selects the empnum element descendants of the employee element children of the context node  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee/descendant::empnum