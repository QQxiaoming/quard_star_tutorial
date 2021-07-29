(: Name: statictyping-5 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses fn:min). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where fn:min(($var,1))
 return $var