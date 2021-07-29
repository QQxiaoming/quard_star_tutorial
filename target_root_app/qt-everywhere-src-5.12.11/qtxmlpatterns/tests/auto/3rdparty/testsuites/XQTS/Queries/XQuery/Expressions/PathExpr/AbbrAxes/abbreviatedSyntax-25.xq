(: Name: abbreviatedSyntax-25 :)
(: Description: Evaluates "employee[@name = condition @type=condition]".  Selects all the employee children of the context node that have both a name attribute and a type attribute. Uses "or" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[@name = "Jane Doe 13" or @type="FT"]