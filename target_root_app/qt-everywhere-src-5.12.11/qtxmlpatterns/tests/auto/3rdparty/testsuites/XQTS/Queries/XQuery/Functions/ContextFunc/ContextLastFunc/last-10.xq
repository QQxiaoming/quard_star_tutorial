(: Name: last-10 :)
(: Description: Evaluation of "fn:last" where the last function is used inside the first predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h//employee[last() = 13][@name = "Jane Doe 13"]