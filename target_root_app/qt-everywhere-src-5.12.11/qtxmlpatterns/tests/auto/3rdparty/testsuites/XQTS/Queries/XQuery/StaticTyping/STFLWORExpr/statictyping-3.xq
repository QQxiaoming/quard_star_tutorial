(: Name: statictyping-3 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses fn:avg). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where fn:avg(($var,1))
 return $var