(: Name: fn-document-uri-2 :)
(: Description: Evaluation of fn:document-uri with argument set to empty sequence. :)
(: Use the fn:count to avoid the empty file. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri(()))