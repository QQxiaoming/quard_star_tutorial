(: Name: statictyping-7 :)
(: Description: Evaluation of static typing feature within a "where" clause.  Wrong operand for expression (uses fn:boolean). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := ("a","b","c")
where fn:boolean($var) = fn:true()
 return $var