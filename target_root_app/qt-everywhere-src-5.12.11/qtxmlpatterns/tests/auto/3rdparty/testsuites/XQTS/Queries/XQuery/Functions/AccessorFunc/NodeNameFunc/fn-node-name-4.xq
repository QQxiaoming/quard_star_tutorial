(: Name: fn-node-name-4 :)
(: Description: Evaluation of node-name function with argument set to an element node. :)
(: use local-name-from-QName to get the local part. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:local-name-from-QName(fn:node-name(<shoe size = "5"/>))