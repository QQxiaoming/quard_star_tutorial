(: Name: fn-id-dtd-10 :)
(: Description: Evaluation of fn:id with multiple IDREF, but none matching one element. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:count(fn:id("nomatching1 nomatching2", exactly-one($input-context1/IDS[1])))
