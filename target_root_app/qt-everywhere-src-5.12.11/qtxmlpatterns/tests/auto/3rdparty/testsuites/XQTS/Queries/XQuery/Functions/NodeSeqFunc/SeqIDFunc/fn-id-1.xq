(: Name: fn-id-1 :)
(: Description: Evaluation of fn:id with incorrect arity. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id("argument 1",<element1>contenty</element1>,"Argument 3")