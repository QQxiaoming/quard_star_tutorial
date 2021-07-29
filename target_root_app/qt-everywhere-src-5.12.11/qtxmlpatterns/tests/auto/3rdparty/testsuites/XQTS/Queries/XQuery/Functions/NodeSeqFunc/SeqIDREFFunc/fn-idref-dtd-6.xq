(: Name: fn-idref-dtd-6 :)
(: Description: Evaluation of fn:idref with given ID not matching a single element. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:idref("nomatchingid", $input-context1/IDS[1]))