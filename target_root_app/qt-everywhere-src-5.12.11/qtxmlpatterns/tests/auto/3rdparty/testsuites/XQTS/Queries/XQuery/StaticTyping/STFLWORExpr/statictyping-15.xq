(: Name: statictyping-15 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong cardinality for operator (uses lt operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := ("a","b","c")
where ($var lt 1)
 return $var