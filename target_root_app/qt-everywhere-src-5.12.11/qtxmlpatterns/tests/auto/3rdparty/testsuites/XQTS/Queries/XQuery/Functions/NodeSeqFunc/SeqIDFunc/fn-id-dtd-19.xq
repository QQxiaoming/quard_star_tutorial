(: Name: fn-id-dtd-19 :)
(: Description: Evaluation of fn:id for which $arg uses the fn:concat function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:concat("i","d1"), $input-context1/IDS[1])