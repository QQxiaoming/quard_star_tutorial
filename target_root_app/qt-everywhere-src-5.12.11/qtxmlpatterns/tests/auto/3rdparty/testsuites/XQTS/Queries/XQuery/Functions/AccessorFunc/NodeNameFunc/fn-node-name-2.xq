(: Name: fn-node-name-2 :)
(: Description: Evaluation of node-name function with argument set to comment node. :)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:node-name($input-context1/works[1]/employee[2]/child::text()[last()]))