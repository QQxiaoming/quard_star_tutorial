(: Name: position-22 :)
(: Description: Evaluation of "fn:position" used in the middle of a path expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[position() = 12]/overtime