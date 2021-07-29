(: Name: fn-document-uri-9 :)
(: Description: Evaluation of fn:document-uri with argument set to a directly constructed comment node. :)
(: Use the fn:count to avoid the empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri(<!-- A comment node -->))