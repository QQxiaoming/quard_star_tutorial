(: Name: abbreviatedSyntax-5 :)
(: Description: Evaluates "employee[1]". Selects the first employee child of the context node. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[1]