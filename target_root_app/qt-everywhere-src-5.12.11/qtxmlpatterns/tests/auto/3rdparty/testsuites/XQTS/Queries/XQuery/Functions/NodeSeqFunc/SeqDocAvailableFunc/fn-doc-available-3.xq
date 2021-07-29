(: Name: fn-doc-available-3 :)
(: Description: Check that a document with duplicated xml:id attributes is flagged. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)
fn:doc-available($input-context)
