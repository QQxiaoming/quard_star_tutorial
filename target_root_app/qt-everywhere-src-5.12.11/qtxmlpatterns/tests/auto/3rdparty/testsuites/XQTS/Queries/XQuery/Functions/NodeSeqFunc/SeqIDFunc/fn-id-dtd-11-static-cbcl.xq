(: Name: fn-id-dtd-11 :)
(: Description: Evaluation of fn:id with IDREF set to empty string. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:count(fn:id("", exactly-one($input-context1/IDS[1])))
