(: Name: fn-id-dtd-5 :)
(: Description: Evaluation of fn:id with given IDREF matching a single element. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

 fn:id("id1", exactly-one($input-context1/IDS[1]))
