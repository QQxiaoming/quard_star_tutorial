(: Name: abbreviatedSyntax-1 :)
(: Description: Evaluates "hours".  Selects the "hours" element children of the context node.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[4]) 
 return $h/hours