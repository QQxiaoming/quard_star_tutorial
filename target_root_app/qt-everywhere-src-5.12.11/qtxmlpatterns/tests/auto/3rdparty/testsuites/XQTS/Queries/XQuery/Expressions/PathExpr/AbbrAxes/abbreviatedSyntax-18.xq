(: Name: abbreviatedSyntax-18 :)
(: Description: Evaluates "employee[5][@gender="female"]".  Selects the fifth employee child of the context node if that child has a gender attribute with value "female". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[5][@gender="female"]