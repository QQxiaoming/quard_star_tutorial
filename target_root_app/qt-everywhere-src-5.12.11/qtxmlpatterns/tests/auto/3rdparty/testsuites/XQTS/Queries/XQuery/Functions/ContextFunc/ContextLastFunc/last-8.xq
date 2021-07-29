(: Name: last-8 :)
(: Description: Evaluation of "fn:last" where two last functions are used inside the predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]) 
  return $h/overtime[last() = last()]