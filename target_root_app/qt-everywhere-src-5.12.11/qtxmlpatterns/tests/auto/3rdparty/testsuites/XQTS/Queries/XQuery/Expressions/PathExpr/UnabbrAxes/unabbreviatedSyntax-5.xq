(: Name: unabbreviatedSyntax-5 :)
(: Description: Evaluate selecting all the children the context node (child::node). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[2]) 
 return $h/child::node()