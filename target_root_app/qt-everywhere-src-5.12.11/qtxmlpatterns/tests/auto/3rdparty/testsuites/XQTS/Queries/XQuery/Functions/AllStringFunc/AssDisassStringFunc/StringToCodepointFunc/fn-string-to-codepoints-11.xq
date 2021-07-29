(: Name: fn-string-to-codepoints-11 :)
(: Description: Evaluation of an "fn:string-to-codepoints" that is used as argument to fn:empty function . :)
(: return false :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:empty(fn:string-to-codepoints("A String"))