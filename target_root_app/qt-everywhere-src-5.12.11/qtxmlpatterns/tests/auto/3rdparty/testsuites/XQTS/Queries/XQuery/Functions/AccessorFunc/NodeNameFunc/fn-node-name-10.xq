(: Name: fn-node-name-10 :)
(: Description: Evaluation of node-name function with argument set to a computed constructed element node with no namespace declaration and one child.:)
(: Use fn:local-name-from-qName to retrieve local part.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:local-name-from-QName(fn:node-name(element elementName { element achild {"some text"}}))