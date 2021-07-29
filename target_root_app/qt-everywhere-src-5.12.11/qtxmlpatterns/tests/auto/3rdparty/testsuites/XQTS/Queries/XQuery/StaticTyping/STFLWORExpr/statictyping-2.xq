(: Name: statictyping-2 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where fn:abs(($var))
 return $var