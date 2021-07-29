(: Name: fn-namespace-uri-10:)
(: Description: Evaluation of the fn:namespace-uri function argument set to a directly constructed element node with no namespace.:)
(: Use the fn:count function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri(<elementNode>with no namespace</elementNode>))
