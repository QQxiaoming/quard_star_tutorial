(: Name: abbreviatedSyntax-21 :)
(: Description: Evaluates "employee[@name and @type]".  Selects all the employee children of the context node that have both a name attribute and a type attribute. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[@name and @type]