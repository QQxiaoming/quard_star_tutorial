(: Name: fn-doc-23 :)
(: Description: Evaluation of fn:doc on a document with complex whitespace. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)
fn:doc($input-context)
