(: Name: fn-doc-9 :)
(: Description: Evaluation of fn:doc, where argument uses fn:lower-case to build the argument.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc(fn:lower-case($input-context))//day