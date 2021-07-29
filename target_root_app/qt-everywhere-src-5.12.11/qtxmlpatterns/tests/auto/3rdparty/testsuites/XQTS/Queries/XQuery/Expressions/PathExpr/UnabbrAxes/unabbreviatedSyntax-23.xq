(: Name: unabbreviatedSyntax-23 :)
(: Description: Evaluate "child::hours[fn:position() > 1]".  Selects all the hours children of the context node other than the first hours child of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee) 
 return $h/child::hours[fn:position() > 1]