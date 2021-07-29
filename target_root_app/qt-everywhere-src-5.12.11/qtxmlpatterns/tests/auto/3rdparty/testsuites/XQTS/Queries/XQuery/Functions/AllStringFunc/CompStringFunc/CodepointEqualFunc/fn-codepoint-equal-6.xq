(: Name: fn-codepoint-equal-6 :)
(: Description: Evaluation of an "fn:codepoint-equal" as argument to fn:not. Returns true :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(fn:codepoint-equal("a","b"))