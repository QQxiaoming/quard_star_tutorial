(: Name: position-10 :)
(: Description: Evaluation of "fn:position" where the position function is used inside the first predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[position() = 1][@name]