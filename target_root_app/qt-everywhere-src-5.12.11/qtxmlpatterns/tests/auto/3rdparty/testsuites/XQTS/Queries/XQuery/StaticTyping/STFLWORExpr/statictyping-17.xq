(: Name: statictyping-17 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong cardinality for operator (uses ne operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := ("a","b","c")
where ($var ne 1)
 return $var