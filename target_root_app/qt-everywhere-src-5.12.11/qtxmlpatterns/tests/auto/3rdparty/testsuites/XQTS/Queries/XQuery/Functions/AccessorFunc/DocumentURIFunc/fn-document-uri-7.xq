(: Name: fn-document-uri-7 :)
(: Description: Evaluation of fn:document-uri with argument set to a computed PI node. :)
(: Use the fn:count to avoid the empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri(processing-instruction {"PITarget"} {"PIContent"}))