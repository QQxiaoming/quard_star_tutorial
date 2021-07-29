(: Name: position-16 :)
(: Description: Evaluation of "fn:position" used together with a division (div) operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[position() = (6 div 2)]