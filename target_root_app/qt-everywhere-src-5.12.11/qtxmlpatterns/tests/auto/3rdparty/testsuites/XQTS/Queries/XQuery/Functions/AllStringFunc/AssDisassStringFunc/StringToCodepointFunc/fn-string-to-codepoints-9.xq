(: Name: fn-string-to-codepoints-9 :)
(: Description: Evaluation of an "fn:string-to-codepoints" that is used as argument to fn:count function . :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:string-to-codepoints("A String"))