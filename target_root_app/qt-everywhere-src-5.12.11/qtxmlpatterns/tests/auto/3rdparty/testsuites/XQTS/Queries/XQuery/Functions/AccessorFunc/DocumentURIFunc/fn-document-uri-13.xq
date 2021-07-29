(: Name: fn-document-uri-13 :)
(: Description: Evaluation of fn:document-uri with argument set to element node from xml file. :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:document-uri($input-context1/works[1]/employee[1]))