(: Name: position-4 :)
(: Description: Evaluation of "fn"position" together with "fn:last". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[position() = last()]