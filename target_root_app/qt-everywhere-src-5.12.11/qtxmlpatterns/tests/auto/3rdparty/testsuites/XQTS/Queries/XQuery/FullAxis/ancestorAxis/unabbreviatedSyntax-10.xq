(: Name: unabbreviatedSyntax-10 :)
(: Description: Evaluate selecting an ancestor (ancestor::employee)- Select the "employee" ancestors of the context node:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[1]/hours) 
 return $h/ancestor::employee