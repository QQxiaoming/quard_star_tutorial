(: Name: fn-local-name-2:)
(: Description: Evaluation of the fn:local-name function with context item not a node.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1 to 100)[fn:local-name()]
