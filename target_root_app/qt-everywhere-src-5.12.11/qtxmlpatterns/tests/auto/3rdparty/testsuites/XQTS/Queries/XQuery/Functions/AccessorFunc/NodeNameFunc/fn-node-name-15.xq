(: Name: fn-node-name-15 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed document node with value set to the empty string.:)
(: Use fn:namspace-uri-from-qName to retrieve local name and fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri-from-QName(fn:node-name(document {""})))