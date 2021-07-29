(: Name: fn-document-uri-17 :)
(: Description: Evaluation of fn:document-uri used as argument to an fn:lower-case function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lower-case(fn:string(fn:contains(fn:document-uri(fn:doc($input-context1)),$input-context1)))