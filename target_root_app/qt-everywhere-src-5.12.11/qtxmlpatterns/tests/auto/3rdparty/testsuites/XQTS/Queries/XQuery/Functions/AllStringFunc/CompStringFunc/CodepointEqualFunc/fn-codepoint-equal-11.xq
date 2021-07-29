(: Name: fn-codepoint-equal-11 :)
(: Description: Evaluation of an "fn:codepoint-equal" with wrong argument type (only second argument). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoint-equal("aa",xs:integer(1))