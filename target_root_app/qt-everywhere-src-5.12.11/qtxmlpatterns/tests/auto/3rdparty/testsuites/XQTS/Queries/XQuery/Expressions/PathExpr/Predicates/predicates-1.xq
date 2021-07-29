(: Name: predicates-1:)
(: Description: Evaluation of a simple predicate with a "true" value (uses "fn:true").:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//integer[fn:true()])

