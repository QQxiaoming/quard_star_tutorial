(: Name: last-7 :)
(: Description: Evaluation of "fn:last" together with the "is" node operator :)
(: casted to integer inside the predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[last()] is $h/employee[last()]