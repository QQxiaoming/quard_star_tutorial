(: Name: fn-document-uri-11 :)
(: Description: Evaluation of fn:document-uri with argument set to a directly constructed document node. :)
(: Use the fn:count to avoid the empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri(document {<anElement>element content</anElement>}))