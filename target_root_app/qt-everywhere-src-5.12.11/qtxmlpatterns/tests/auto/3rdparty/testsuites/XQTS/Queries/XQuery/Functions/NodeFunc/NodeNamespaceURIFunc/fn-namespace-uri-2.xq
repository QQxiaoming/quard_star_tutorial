(: Name: fn-namespace-uri-2:)
(: Description: Evaluation of the fn:namespace-uri function with more than one argument.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:namespace-uri(<elementNode>some context</elementNode>,"A Second Argument")
