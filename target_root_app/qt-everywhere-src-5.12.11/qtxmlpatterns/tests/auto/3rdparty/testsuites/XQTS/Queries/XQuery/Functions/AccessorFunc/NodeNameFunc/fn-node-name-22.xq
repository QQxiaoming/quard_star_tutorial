(: Name: fn-node-name-22 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed processing instruction node.:)
(: Use fn:fn-local-name-from-QName to retrieve local name. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:local-name-from-QName(fn:node-name(processing-instruction piName {"Processing Instruction content"}))