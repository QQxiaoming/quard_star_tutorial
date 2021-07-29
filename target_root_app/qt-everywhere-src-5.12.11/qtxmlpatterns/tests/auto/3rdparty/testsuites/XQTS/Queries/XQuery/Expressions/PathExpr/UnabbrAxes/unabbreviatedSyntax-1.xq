(: Name: unabbreviatedSyntax-1 :)
(: Description: Evaluate the child axis of the context node (child::empnum)  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee) 
 return $h/child::empnum