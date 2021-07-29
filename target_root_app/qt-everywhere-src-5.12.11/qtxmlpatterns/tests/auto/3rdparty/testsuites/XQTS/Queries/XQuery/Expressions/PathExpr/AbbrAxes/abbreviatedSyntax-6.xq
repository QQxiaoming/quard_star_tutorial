(: Name: abbreviatedSyntax-6 :)
(: Description: Evaluates "para[fn:last()]". Selects the last employee child of the context node. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[fn:last()]