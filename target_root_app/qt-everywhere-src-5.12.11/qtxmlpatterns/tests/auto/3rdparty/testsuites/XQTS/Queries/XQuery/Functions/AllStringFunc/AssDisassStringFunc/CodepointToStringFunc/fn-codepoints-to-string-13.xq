(: Name: fn-codepoints-to-string-13 :)
(: Description: Evaluation of an "fn:codepoints-to-string" that is used as argument to fn:concat function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:concat(fn:codepoints-to-string((49,97)),"1a")