(:*******************************************************:)
(:Name: statictyping-24                                  :)
(:Written By: Sorin Nasoi                                :)
(:Description: Evaluation of static typing feature within:)
(:a "where" clause.                                      :)
(:Wrong operand for expression (uses fn:not)             :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := ("a","b","c")
where fn:not($var) eq fn:true()
return $var