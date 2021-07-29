(: Name: fn-node-name-11 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed attribute node with value set to empty string.:)
(: Use fn:local-name-from-qName to retrieve local name. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:local-name-from-QName(fn:node-name(attribute attributeName {""}))