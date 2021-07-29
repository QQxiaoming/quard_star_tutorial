(: Name: last-6 :)
(: Description: Evaluation of "fn:last" together with the "<<" node operator :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[last()] << $h/employee[last()]