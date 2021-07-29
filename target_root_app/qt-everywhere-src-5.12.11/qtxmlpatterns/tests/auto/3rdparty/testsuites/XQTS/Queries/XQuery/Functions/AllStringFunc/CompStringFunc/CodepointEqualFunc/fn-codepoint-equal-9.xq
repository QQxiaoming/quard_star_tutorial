(: Name: fn-codepoint-equal-9 :)
(: Description: Evaluation of an "fn:codepoint-equal" whose arguments use fn:string() for a string. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoint-equal(fn:string("aa"),fn:string("aa"))