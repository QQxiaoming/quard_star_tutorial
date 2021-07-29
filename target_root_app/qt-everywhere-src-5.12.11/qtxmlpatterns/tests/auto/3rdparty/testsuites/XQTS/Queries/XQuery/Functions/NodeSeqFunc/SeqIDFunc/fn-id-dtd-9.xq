(: Name: fn-id-dtd-9 :)
(: Description: Evaluation of fn:id with multiple IDREF, but only one matching one element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id("id1 nomatching", $input-context1/IDS[1])