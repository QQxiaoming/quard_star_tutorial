(: Name: position-13 :)
(: Description: Evaluation of "fn:position" used together with an addition operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[position() = (2 + 2)]