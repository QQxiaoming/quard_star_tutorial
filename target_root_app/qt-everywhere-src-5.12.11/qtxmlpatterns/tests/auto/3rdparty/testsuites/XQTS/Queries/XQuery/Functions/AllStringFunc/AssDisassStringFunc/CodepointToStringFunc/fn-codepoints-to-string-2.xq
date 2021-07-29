(: Name: fn-codepoints-to-string-2:)
(: Description: Invalid XML character codepoint as part of "code-points-to-string" function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoints-to-string(10000000)