(: Name: fn-namespace-uri-15:)
(: Description: Evaluation of the fn:namespace-uri function argument set to an attribute node with no namespace queried from a file.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri($input-context1/works[1]/employee[1]/@name))