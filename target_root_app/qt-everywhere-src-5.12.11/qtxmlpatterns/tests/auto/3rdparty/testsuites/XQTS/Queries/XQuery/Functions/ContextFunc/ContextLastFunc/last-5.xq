(: Name: last-5 :)
(: Description: Evaluation of "fn:last" together with "fn:position" (format last() = position()). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[last() = position()]