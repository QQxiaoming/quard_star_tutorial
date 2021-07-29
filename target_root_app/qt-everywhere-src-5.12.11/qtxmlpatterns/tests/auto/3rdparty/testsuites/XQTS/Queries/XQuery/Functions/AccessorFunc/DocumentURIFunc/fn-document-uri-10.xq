(: Name: fn-document-uri-10 :)
(: Description: Evaluation of fn:document-uri with argument set to a directly constructed element node. :)
(: Use the fn:count to avoid the empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri(<anElement>element content</anElement>))