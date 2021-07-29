(: Name: fn-id-dtd-9 :)
(: Description: Evaluation of fn:id with multiple IDREF, but only one matching one element. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:id("id1 nomatching", exactly-one($input-context1/IDS[1]))
