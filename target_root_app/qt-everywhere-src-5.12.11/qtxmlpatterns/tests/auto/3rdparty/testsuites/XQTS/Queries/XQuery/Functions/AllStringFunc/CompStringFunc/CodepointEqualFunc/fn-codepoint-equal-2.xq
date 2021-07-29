(: Name: fn-codepoint-equal-2 :)
(: Description: Evaluation of an "fn:codepoint-equal" with arguments set to empty sequence :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:codepoint-equal((),()))