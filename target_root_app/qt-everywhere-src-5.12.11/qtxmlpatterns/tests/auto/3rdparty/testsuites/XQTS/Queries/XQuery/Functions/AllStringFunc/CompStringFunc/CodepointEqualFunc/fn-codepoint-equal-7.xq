(: Name: fn-codepoint-equal-7 :)
(: Description: Evaluation of an "fn:codepoint-equal" as argument to fn:not. Returns false :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(fn:codepoint-equal("a","a"))