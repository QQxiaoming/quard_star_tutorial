(: Name: fn-idref-dtd-11 :)
(: Description: Evaluation of fn:idref with ID set to empty string. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:idref("", $input-context1/IDS[1]))