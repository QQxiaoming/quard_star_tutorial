(: Name: statictyping-14 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong cardinality for operator (uses le operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := ("a","b","c")
where ($var le 1)
 return $var