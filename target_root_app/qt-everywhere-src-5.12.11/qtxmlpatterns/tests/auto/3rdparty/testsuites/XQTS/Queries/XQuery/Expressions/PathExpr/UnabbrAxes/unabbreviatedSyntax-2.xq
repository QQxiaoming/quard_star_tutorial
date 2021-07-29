(: Name: unabbreviatedSyntax-2 :)
(: Description: Evaluate selecting all element children of the context node:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[1]) 
 return $h/child::*