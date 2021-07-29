(: Name: fn-node-name-21 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed text node with value set to a non empty value.:)
(: Use fn:namespace-uri-from-QName to retrieve namespace and fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri-from-QName(fn:node-name(text {"a text value"})))