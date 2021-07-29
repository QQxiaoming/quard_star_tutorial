(: Name: abbreviatedSyntax-9 :)
(: Description: Evaluates "employee//hours".  Selects the hours element descendants of the employee element children of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee//hours