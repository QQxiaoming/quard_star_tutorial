(: Name: abbreviatedSyntax-14 :)
(: Description: Evaluates "..".  Selects the parent of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]/overtime) 
 return $h/..