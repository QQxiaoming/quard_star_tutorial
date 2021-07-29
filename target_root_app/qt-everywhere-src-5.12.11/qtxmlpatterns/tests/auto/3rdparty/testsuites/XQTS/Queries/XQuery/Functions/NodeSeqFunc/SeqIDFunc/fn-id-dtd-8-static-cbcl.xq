(: Name: fn-id-dtd-8 :)
(: Description: Evaluation of fn:id with multiple IDREF matching multiple elements. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:id("id1 id2", exactly-one($input-context1/IDS[1]))
