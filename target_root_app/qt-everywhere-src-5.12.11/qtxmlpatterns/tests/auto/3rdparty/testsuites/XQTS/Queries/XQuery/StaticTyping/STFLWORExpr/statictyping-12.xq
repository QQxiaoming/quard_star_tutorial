(: Name: statictyping-12 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses "mod" operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where ($var mod 2) = 3
 return $var