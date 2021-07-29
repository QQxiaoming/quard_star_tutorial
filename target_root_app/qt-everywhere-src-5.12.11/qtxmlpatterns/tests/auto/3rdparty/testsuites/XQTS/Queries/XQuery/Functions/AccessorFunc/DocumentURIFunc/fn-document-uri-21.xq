(: Name: fn-document-uri-21 :)
(: Description: Evaluation of fn:document-uri used as argument to an fn:substring-after function. :)
(: Use string. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring-after(fn:string(fn:contains(fn:document-uri($input-context1),"works-mod")),"t")