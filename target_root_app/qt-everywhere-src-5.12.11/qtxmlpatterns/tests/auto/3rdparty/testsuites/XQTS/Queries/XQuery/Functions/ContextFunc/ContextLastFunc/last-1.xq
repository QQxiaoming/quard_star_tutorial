(: Name: last-1 :)
(: Description: Evaluation of "fn:last", where context function is an element node and the position is last. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h//employee[last()]