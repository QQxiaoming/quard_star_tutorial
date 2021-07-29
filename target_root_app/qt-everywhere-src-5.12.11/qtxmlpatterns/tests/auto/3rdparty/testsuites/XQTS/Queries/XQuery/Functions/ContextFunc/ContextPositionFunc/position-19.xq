(: Name: position-19 :)
(: Description: Evaluation of "fn:position" used together with a logical expression ("and" operator and "false" function). :)
(: Use fn:count to avoid empty file.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]) 
  return fn:count($h/overtime[position() and fn:false()])