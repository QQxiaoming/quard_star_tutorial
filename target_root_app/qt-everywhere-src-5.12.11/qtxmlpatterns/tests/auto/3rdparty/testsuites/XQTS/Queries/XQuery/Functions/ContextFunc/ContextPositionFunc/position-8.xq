(: Name: position-8 :)
(: Description: Evaluation of "fn"position" where two position functions are used inside the predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]) 
  return $h/overtime[position() = position()]