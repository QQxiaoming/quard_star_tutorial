(: Name: fn-node-name-19 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed text node with value set to the the empty string.:)
(: Use fn:namespace-uri-from-qName to retrieve namespace and fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri-from-QName(fn:node-name(text {""})))