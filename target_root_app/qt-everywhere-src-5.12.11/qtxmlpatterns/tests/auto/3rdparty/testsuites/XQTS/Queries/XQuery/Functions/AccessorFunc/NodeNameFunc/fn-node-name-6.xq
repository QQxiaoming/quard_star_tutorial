(: Name: fn-node-name-6 :)
(: Description: Evaluation of node-name function with argument set to a Processing Instruction node. :)
(: Use local-name-from-QName to get local part :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:local-name-from-QName(fn:node-name(<?format role="output" ?>))