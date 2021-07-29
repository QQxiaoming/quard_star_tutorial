(: Name: statictyping-13 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong cardinality for operator (uses eq operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := ("a","b","c")
where ($var eq 1)
 return $var