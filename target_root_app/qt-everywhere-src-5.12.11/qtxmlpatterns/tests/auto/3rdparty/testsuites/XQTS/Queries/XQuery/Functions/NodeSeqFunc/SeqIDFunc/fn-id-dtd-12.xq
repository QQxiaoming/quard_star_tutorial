(: Name: fn-id-dtd-12 :)
(: Description: Evaluation of fn:id function that give first argument as argument to fn:substring. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:substring("1id3",2), $input-context1/IDS[1])