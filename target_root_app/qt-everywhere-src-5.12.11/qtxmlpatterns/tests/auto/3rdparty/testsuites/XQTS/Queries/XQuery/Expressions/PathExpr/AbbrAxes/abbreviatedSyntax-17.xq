(: Name: abbreviatedSyntax-17 :)
(: Description: Evaluates "employee[@gender="female"][5]". Selects the fifth element child of the context node that has a gender attribute with value "female". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[@gender="female"][5]