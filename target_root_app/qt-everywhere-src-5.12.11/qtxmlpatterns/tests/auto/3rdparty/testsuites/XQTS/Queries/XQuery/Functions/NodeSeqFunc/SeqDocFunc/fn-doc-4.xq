(: Name: fn-doc-4 :)
(: Description: Evaluation of ana fn:doc with argument set to empty sequence.:)
(: Uses fn:count to avoid empty sequence. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:doc(()))