(: Name: fn-node-name-14 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed document node with value set to the empty string.:)
(: Use fn:local-name-from-qName to retrieve local name and fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:local-name-from-QName(fn:node-name(document {""})))