(: Name: fn-id-dtd-13 :)
(: Description: Evaluation of fn:id, where the same IDREF makes refence to the same element. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:id("id4", exactly-one($input-context1/IDS[1]))
