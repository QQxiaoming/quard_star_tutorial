(: Name: position-1 :)
(: Description: Evaluation of "fn"position", where context function is an element node and position is first. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[position() = 1]