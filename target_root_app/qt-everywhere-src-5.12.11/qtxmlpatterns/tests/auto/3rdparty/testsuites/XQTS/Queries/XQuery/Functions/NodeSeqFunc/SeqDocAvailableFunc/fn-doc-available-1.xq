(: Name: fn-doc-available-1 :)
(: Description: Evaluation of ana fn:doc-available function with wrong arity.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc-available("http://example.com","string 2")