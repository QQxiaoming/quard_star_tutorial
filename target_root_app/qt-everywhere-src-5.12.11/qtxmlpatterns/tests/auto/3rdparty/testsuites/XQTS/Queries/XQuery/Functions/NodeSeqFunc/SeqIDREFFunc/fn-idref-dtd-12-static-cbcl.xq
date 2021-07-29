(: Name: fn-idref-dtd-12 :)
(: Description: Evaluation of fn:idref used as ar argument to function fn:node-name() :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:node-name(exactly-one(fn:idref("id2", exactly-one($input-context1))))
