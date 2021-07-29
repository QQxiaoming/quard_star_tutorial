(: Name: fn-doc-11 :)
(: Description: Evaluation of fn:doc, where argument uses fn:substring to build the argument.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc(fn:substring($input-context,1))//day