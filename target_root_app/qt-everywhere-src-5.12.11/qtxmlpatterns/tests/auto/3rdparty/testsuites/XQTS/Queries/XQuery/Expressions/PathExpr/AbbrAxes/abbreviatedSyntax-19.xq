(: Name: abbreviatedSyntax-19 :)
(: Description: Evaluates "employee[status="active"]".  Selects the employee children of the context node that have one or more status children whose typed value is equal to the string "active". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[status="active"]