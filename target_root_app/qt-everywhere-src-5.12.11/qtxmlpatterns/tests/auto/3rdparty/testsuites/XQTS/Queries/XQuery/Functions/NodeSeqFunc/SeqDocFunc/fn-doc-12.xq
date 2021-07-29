(: Name: fn-doc-12 :)
(: Description: Evaluation of fn:doc, where argument uses fn:concat (and empty string) to build the argument.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc(fn:concat($input-context,""))//day