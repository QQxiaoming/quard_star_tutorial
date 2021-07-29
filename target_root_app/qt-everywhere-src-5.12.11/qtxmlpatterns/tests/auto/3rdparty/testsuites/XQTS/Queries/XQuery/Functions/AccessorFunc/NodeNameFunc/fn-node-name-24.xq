(: Name: fn-node-name-24 :)
(: Description: Evaluation of node-name function with wrong number of arguments.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:node-name(processing-instruction piName {"Processing Instruction content"},"A Second Argument")