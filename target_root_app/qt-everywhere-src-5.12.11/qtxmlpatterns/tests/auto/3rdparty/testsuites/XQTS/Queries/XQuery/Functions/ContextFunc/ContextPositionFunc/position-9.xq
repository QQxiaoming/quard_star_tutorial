(: Name: position-9 :)
(: Description: Evaluation of "fn:position" where the position function is used inside a second predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[@name][position() = 1]