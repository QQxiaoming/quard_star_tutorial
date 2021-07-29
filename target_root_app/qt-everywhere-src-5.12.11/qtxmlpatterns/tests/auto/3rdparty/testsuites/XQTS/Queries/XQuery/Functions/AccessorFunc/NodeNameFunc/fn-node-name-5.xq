(: Name: fn-node-name-5 :)
(: Description: Evaluation of node-name function with argument set to comment node. :)
(: Use of fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:node-name(<!-- This a comment node -->))