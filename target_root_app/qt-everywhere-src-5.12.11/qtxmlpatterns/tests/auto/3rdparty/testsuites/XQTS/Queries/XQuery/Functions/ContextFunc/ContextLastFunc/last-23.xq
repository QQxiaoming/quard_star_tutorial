(: Name: last-23 :)
(: Description: Evaluation of "last()" within a positional predicate :)
(: Author: Michael Kay :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
  return $h/employee[position() = 5 to last()]/@name/string()