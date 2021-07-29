(: Name: abbreviatedSyntax-2 :)
(: Description: Evaluates "text()".  Selects all text node children of the context node. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[2]) 
 return $h/text()