(: Name: fn-idref-dtd-12 :)
(: Description: Evaluation of fn:idref used as ar argument to function fn:node-name() :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:node-name(fn:idref("id2", $input-context1/IDS[1]))