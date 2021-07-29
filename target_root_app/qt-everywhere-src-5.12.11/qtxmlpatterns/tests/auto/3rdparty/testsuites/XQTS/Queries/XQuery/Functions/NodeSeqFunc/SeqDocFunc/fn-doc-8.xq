(: Name: fn-doc-8 :)
(: Description: Evaluation of fn:doc, where argument uses fn:upper-case/lower-case to build the argument.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc(fn:lower-case(fn:upper-case($input-context)))//day