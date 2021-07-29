(: Name: fn-doc-available-4 :)
(: Description: Check that a document with an invalid xml:id attribute is flagged. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc-available($input-context)
