(: Name: position-14 :)
(: Description: Evaluation of "fn:position" used together with a subtraction operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[position() = (5 - 2)]