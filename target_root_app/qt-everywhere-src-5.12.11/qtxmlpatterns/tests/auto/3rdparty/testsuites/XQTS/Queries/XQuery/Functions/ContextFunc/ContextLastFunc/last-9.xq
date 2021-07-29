(: Name: last-9 :)
(: Description: Evaluation of "fn:last" where the last function is used inside a second predicate. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h//employee[@name="Jane Doe 13"][last() = 1]