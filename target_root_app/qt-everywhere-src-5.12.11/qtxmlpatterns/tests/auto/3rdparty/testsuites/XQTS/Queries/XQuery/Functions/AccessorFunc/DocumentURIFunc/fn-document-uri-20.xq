(: Name: fn-document-uri-20 :)
(: Description: Evaluation of fn:document-uri used as argument to an fn:substring-before function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring-before(fn:string(fn:contains(fn:document-uri($input-context1),"works-mod")),"e")