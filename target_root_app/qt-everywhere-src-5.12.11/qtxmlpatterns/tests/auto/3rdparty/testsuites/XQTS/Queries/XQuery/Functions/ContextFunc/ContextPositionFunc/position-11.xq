(: Name: position-11 :)
(: Description: Evaluation of "fn:position" used in conjucntion with the fn:not function :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]/overtime) 
  return $h/day[not(position() = 1)]