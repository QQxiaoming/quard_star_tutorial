(: Name: fn-codepoints-to-string-9 :)
(: Description: Evaluation of an "fn:codepoints-to-string" used as argument to the "fn:upper-case" function and use codepoints 65,32,83,84,82,73,78,71 (string "A STRING"). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:upper-case(fn:codepoints-to-string((65,32,83,84,82,73,78,71)))