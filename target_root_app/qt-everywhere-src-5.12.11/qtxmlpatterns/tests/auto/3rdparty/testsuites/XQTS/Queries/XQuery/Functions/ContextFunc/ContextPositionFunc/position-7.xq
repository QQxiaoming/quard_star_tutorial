(: Name: position-7 :)
(: Description: Evaluation of "fn"position" together with a variable of type string :)
(: casted to integer inside the predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in "1" return
for $h in ($input-context1/works) 
  return $h/employee[position() = xs:integer($var)]