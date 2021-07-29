(: Name: unabbreviatedSyntax-31 :)
(: Description: Evaluate "child::employee[child::status]".  Selects the employee children of the context node that have one or more status children. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee[child::status]