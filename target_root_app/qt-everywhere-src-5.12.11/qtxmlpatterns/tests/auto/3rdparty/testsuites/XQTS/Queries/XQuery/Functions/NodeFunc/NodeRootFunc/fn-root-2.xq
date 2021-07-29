(: Name: fn-root-2:)
(: Description: Evaluation of the fn:root function with no arguments that uses context node, which is not a node.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1 to 100)[fn:root()]