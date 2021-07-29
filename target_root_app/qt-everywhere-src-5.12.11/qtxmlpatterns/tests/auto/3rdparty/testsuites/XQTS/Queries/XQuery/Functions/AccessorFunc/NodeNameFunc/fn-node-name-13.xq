(: Name: fn-node-name-13 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed attribute node with value set to a non empty value.:)
(: Use fn:namespace-uri-from-qName to retrieve local name and fn:count. to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri-from-QName(fn:node-name(attribute attributeName {"an attribute value"})))