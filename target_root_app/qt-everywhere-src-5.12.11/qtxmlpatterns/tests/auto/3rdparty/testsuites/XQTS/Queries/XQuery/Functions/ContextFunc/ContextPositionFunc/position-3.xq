(: Name: position-3 :)
(: Description: Evaluation of "fn"position", where the last item is selected. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/employee[position() = 13]