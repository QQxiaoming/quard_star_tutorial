(: Name: fn-doc-17 :)
(: Description: Evaluation of fn:doc with argument set to an invalid URI.:)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc("%gg")
