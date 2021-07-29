(: Name: fn-base-uri-11 :)
(: Description: Evaluation of base-uri function with argument set to a computed constructed Document node with no base-xml argument. :)
(: Use fn:count :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(document {<aDocument>some content</aDocument>}))