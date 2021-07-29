(: Name: fn-codepoint-equal-8 :)
(: Description: Evaluation of an "fn:codepoint-equal" whose arguments use fn:string() for a number :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoint-equal(fn:string(1),fn:string(1))