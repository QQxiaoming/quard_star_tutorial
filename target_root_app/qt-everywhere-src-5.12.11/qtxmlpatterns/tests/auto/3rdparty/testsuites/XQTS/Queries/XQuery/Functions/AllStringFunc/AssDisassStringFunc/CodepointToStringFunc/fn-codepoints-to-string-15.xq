(: Name: fn-codepoints-to-string-15 :)
(: Description: Evaluation of an "fn:codepoints-to-string" that is used as argument to fn:string-length function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:codepoints-to-string((49,97)))