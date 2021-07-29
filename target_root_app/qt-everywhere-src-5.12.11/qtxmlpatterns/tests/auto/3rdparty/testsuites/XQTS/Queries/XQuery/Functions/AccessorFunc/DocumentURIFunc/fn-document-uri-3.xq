(: Name: fn-document-uri-3 :)
(: Description: Evaluation of fn:document-uri with argument set to a computed element node. :)
(: Use the fn:count to avoid the empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri(element anElement {"some content"}))