(: Name: fn-node-name-9 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed element node with no namespace declaration and no children.:)
(: Use fn:namespace-uri-from-qName to retrieve namespace. Should return empty string. :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri-from-QName(fn:node-name(element elementName {})))