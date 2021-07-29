(: Name: position-12 :)
(: Description: Evaluation of "fn:position" as a predicate to a wildcard (*). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[12]/overtime) 
  return $h/*[position() = position()]