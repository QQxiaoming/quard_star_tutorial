(: Name: fn-doc-7 :)
(: Description: Typical usage of fn:doc.  Retrieve a part of the resources.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc($input-context)//day