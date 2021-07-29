(: Name: fn-codepoints-to-string-14 :)
(: Description: Evaluation of an "fn:codepoints-to-string" that is used as argument to fn:string-to-codepoints function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-to-codepoints(fn:codepoints-to-string((49,97)))