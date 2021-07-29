(: Name: fn-document-uri-4 :)
(: Description: Evaluation of fn:document-uri with argument set to a computed comment node. :)
(: Use the fn:count to avoid the empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri(comment {"a comment node"}))