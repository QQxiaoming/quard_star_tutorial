(: Name: last-3 :)
(: Description: Evaluation of "fn:last", where the first item is selected. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]) 
 return $h/overtime[last() = 1]