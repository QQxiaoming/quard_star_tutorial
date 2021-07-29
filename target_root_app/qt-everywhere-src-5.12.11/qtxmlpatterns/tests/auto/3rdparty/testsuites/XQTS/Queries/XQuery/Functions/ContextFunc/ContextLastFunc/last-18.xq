(: Name: position-18 :)
(: Description: Evaluation of "fn:position" used together with a logical expression ("and" operator and "true" function). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]) 
  return ($h/overtime[position() and fn:true()])