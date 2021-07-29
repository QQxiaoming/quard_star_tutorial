(: Name: abbreviatedSyntax-20 :)
(: Description: Evaluates "employee[overtime]".  Selects the employee children of the context node that have one or more overtime children. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[overtime]