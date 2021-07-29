(: Name: position-6 :)
(: Description: Evaluation of "fn"position" together with a variable of type integer :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in 1 return
for $h in ($input-context1/works) 
  return $h/employee[position() = $var]