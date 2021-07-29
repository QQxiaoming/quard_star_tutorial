(: Name: statictyping-20 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong cardinality for operator (uses "*" operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := (1,2,3)
where ($var * 1)
 return $var