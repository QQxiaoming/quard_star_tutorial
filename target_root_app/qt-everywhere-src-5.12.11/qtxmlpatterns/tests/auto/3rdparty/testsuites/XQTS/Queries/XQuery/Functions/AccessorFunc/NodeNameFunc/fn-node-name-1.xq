(: Name: fn-node-name-2 :)
(: Description: Evaluation of node function with argument set empty sequence. :)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:node-name(()))