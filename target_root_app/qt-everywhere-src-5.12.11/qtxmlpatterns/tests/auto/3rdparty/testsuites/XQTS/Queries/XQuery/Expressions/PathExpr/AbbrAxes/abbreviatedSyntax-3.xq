(: Name: abbreviatedSyntax-3 :)
(: Description: Evaluates "@name". Selects the name attribute of the context node. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[10]) 
 return $h