(: Name: fn-doc-13 :)
(: Description: Evaluation of fn:doc, where argument uses fn:concat (and empty sequence) to build the argument.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc(fn:concat($input-context,()))//day