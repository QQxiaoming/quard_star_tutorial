(: Name: predicatesns-3:)
(: Description: Evaluation of a simple predicate with a "true" value (uses "fn:false" and fn:not()). :)
(: Not Schema dependent. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//integer[fn:not(fn:false())])

