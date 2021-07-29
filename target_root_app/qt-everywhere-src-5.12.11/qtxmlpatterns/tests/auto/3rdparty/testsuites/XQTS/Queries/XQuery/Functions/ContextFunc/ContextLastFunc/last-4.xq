(: Name: last-4 :)
(: Description: Evaluation of "fn:last" together with "fn:position". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[position() = last()]