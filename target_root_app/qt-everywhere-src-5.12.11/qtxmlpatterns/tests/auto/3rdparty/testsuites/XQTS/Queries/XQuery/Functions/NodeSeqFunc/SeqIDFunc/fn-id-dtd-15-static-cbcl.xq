(: Name: fn-id-dtd-15 :)
(: Description: Evaluation of fn:id for which IDREF list have the same value for all its members. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:id("id1 id1", exactly-one($input-context1/IDS[1]))
