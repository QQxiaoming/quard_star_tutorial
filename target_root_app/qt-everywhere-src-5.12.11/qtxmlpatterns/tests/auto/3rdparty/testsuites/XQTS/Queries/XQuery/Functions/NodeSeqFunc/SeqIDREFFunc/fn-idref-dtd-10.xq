(: Name: fn-idref-dtd-10 :)
(: Description: Evaluation of fn:idref with multiple ID, but none matching one element. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:idref("nomatching1 nomatching2", $input-context1/IDS[1]))