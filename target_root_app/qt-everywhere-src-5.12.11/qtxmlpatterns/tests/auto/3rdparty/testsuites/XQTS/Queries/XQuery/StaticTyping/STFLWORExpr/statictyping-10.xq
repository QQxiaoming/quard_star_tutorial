(: Name: statictyping-10 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses "div" operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where ($var div 2) = 3
 return $var