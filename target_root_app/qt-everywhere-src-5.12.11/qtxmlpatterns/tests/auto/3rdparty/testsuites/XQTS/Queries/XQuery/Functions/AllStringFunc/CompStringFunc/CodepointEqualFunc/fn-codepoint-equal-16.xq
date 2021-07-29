(: Name: fn-codepoint-equal-16 :)
(: Description: Evaluation of an "fn:codepoint-equal" as part of boolean expression ("and" and fn:true()). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoint-equal("aa","aa") and fn:true()