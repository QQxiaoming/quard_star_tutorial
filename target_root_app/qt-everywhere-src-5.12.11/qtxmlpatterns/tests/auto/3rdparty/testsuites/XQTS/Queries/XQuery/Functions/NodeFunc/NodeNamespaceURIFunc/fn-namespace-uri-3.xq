(: Name: fn-namespace-uri-3:)
(: Description: Evaluation of the fn:namespace-uri function with the context item not being a node.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1 to 100)[fn:namespace-uri()]