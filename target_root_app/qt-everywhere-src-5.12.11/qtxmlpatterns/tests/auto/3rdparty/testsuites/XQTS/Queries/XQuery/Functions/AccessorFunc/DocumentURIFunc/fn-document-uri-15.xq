(: Name: fn-document-uri-15 :)
(: Description: Evaluation of fn:document-uri used as argument to an fn:string-length function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:string(fn:contains(fn:document-uri(fn:doc($input-context1)),$input-context1)))