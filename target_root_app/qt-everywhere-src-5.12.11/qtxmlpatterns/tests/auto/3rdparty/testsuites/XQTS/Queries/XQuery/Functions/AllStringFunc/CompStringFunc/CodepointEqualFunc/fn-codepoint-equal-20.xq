(: Name: fn-codepoint-equal-20 :)
(: Description: Evaluation of an "fn:codepoint-equal" as part of boolean expression involving two fn:codepoint-equal ("and" operator). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoint-equal("aa","aa") and fn:codepoint-equal("aa","aa")