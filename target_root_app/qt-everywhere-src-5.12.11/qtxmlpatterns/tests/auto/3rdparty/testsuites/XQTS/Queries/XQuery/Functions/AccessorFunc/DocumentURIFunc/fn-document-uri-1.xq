(: Name: fn-document-uri-1 :)
(: Description: Evaluation of fn:document-uri with incorrect arity. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:document-uri(<element1>contenty</element1>,"Argument 2")