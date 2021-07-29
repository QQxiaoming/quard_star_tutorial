(: Name: statictyping-16 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong cardinality for operator (uses gt operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := ("a","b","c")
where ($var gt 1)
 return $var