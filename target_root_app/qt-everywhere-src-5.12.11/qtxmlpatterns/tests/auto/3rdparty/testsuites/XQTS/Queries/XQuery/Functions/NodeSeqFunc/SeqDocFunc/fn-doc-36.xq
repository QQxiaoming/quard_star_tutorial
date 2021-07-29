(: Name: fn-doc-36 :)
(: Description: Count text nodes in a document containing character references. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

count(doc($input-context)//text())
