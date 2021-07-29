(: Name: position-15 :)
(: Description: Evaluation of "fn:position" used together with a multiplication operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[position() = (3 * 2)]