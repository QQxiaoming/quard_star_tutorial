(: Name: statictyping-11 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses "idiv" operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where ($var idiv 2) = 3
 return $var