(: Name: unabbreviatedSyntax-3 :)
(: Description: Evaluate selecting all text node children of the context node:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works[1]/employee[2]) 
 return $h/child::text() 