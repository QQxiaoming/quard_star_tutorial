(: Name: fn-doc-available-2 :)
(: Description: Evaluation of ana fn:doc-available function with wrong argument type.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc-available(xs:integer(2))