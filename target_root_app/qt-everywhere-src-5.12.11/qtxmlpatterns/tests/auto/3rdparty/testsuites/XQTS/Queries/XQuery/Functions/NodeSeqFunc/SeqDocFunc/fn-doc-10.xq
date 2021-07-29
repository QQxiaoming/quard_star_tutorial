(: Name: fn-doc-10 :)
(: Description: Evaluation of fn:doc, where argument uses xs:string to build the argument.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc(xs:string($input-context))//day