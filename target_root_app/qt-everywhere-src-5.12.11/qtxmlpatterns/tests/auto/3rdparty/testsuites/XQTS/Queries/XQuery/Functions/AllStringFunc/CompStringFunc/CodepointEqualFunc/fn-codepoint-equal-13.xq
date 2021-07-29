(: Name: fn-codepoint-equal-13 :)
(: Description: Evaluation of an "fn:codepoint-equal" with arguments set to "aa" and lower-case("AA") respectively. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoint-equal("aa",fn:lower-case("AA"))