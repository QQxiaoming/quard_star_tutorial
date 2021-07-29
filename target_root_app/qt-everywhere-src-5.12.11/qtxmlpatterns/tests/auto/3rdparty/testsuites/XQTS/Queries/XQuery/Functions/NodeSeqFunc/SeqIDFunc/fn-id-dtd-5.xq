(: Name: fn-id-dtd-5 :)
(: Description: Evaluation of fn:id with given IDREF matching a single element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:id("id1", $input-context1/IDS[1])