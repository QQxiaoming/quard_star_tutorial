(: Name: fn-document-uri-12 :)
(: Description: Evaluation of fn:document-uri with argument set to document node from xml file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:contains(fn:document-uri(fn:doc($input-context1)),$input-context1) or (fn:document-uri(fn:doc($input-context1)) = "")