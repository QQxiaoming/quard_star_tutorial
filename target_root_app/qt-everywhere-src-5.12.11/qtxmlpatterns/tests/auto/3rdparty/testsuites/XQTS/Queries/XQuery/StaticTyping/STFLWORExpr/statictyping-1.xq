(: Name: statictyping-1 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ("a","b","c")
where $var + 1 = 3
 return $var