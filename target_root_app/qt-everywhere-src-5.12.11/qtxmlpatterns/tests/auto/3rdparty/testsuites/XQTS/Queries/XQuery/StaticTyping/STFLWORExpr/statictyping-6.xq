(: Name: statictyping-6 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses fn:sum). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where fn:sum(($var,1))
 return $var