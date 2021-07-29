(: Name: predicatesns-4:)
(: Description: Evaluation of a simple predicate set to a boolean expression ("and" operator). :)
(: Not Schema dependent. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//integer[fn:true() and fn:true()])

