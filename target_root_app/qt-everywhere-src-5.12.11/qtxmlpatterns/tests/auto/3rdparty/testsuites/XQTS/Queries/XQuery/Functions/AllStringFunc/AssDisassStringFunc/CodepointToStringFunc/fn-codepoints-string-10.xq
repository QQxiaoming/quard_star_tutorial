(: Name: fn-codepoints-to-string-10 :)
(: Description: Evaluation of an "fn:codepoints-to-string" used as argument to the "fn:lower-case" function and use codepoints 97,32,115,116,114,105,110,103 (String "a string"). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lower-case(fn:codepoints-to-string((97,32,115,116,114,105,110,103)))