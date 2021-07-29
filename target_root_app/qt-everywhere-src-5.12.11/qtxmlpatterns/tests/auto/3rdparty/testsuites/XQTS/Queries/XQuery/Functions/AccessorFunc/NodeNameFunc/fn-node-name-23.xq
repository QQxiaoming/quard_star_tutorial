(: Name: fn-node-name-23 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed processing instruction node.:)
(: Use fn:fn-namespace-uri-from-QName to retrieve namespace and fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri-from-QName(fn:node-name(processing-instruction piName {"Processing Instruction content"})))