(: Name: fn-idref-1 :)
(: Description: Evaluation of fn:idref with incorrect arity. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:idref("argument 1",<element1>contenty</element1>,"Argument 3")