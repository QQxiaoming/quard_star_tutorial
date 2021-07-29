(: Name: fn-string-to-codepoints-14 :)
(: Description: Evaluation of an "fn:string-to-codepoints" that is used as argument to fn:empty function. :)
(: return true :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:exists(fn:string-to-codepoints("A String"))