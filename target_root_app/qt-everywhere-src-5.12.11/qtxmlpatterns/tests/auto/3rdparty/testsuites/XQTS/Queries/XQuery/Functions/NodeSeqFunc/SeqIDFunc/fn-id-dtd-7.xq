(: Name: fn-id-dtd-7 :)
(: Description: Evaluation of fn:id with given IDREF matching same element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id("id2 id2", $input-context1/IDS[1])