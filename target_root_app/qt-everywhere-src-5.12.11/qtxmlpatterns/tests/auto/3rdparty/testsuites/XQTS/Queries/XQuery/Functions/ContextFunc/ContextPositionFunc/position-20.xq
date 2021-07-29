(: Name: position-20 :)
(: Description: Evaluation of "fn:position" used together with a logical expression ("or" operator and "true" function). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]) 
  return $h/overtime[position() or fn:true()]