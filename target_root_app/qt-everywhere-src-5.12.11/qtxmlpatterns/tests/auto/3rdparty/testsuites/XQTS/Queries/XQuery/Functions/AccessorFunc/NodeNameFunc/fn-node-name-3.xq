(: Name: fn-node-name-3 :)
(: Description: Evaluation of node-name function with argument set to an element node. :)
(: Uses local-name-from-QName to get local part :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:local-name-from-QName(fn:node-name($input-context1/works[1]/employee[2]))