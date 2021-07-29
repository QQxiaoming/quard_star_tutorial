(: Name: fn-codepoints-to-string-16 :)
(: Description: Evaluation of an "fn:codepoints-to-string" that is used as argument to fn:string-join function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-join((fn:codepoints-to-string((49,97)),'ab'),'')