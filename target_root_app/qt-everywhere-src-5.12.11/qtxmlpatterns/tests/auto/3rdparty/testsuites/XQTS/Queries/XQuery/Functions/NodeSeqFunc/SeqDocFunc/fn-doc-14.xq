(: Name: fn-doc-14 :)
(: Description: Evaluation of fn:doc, where argument uses fn:normalize-space to build the argument.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc(fn:normalize-space($input-context))//day