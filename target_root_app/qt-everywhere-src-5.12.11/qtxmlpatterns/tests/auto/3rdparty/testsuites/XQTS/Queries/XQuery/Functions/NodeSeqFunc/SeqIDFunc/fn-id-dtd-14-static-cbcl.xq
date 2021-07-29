(: Name: fn-id-dtd-14 :)
(: Description: Evaluation of fn:id for which the given IDREF contains a prefix. :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:count(fn:id("p1:id5", exactly-one($input-context1/IDS[1])))
