(: Name: statictyping-4 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses fn:max). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where fn:max(($var,1))
 return $var