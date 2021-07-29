(: Name: fn-document-uri-16 :)
(: Description: Evaluation of fn:document-uri used as argument to an fn:upper-case function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:upper-case(fn:string(fn:contains(fn:document-uri(fn:doc($input-context1)),$input-context1)))