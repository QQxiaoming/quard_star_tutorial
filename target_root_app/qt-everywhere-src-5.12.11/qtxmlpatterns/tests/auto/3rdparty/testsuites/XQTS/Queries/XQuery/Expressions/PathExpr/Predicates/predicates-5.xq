(: Name: predicates-5:)
(: Description: Evaluation of a simple predicate set to a boolean expression ("or" operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//integer[fn:true() or fn:true()])

