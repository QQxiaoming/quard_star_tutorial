(: Name: fn-node-name-8 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed element node with no namespace declaration and no children.:)
(: Use local-name-from-qName to retrieve local part. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:local-name-from-QName(fn:node-name(element elementName {}))