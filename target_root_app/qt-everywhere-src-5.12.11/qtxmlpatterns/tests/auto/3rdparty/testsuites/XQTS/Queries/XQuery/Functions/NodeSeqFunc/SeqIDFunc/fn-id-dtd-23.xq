(: Name: fn-id-dtd-23 :)
(: Description: Evaluation of fn:id together with declare ordering. :)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id("id1 id2", $input-context1/IDS[1])