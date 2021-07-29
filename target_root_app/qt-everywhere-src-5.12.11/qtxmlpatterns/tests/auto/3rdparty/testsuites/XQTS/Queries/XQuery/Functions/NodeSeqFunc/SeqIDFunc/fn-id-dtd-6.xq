(: Name: fn-id-dtd-6 :)
(: Description: Evaluation of fn:id with given IDREF not matching a single element. :)
(: USes fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:id("nomatchingid", $input-context1/IDS[1]))