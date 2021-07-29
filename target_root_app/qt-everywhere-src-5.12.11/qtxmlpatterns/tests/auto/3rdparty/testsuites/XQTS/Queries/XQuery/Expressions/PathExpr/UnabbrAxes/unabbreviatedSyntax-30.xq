(: Name: unabbreviatedSyntax-30 :)
(: Description: Evaluate "child::employee[child::empnum = 'E3']".  Selects the employee children of the context node that have one or more empnum children whose :)
(: typed value is equal to the string "E3". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee[child::empnum = 'E3']