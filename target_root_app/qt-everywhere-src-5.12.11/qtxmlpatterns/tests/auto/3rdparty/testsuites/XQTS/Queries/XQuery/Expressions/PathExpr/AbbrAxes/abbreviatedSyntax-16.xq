(: Name: abbreviatedSyntax-16 :)
(: Description: Evaluates "employee[@name="Jane Doe 11"]".  Selects all employee children of the context node that have a name attribute with a value "Jane Doe 11". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[@name="Jane Doe 11"]