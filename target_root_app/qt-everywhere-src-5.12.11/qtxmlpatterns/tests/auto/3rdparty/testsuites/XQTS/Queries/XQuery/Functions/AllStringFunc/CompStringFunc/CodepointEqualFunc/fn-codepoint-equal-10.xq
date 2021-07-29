(: Name: fn-codepoint-equal-10 :)
(: Description: Evaluation of an "fn:codepoint-equal" with wrong argument type. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoint-equal(xs:integer(1),xs:integer(1))