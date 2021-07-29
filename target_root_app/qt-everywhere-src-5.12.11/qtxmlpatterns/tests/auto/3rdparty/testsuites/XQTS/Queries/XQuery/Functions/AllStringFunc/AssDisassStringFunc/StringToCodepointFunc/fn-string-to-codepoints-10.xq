(: Name: fn-string-to-codepoints-10 :)
(: Description: Evaluation of an "fn:string-to-codepoints" that is used as argument to fn:avg function . :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:avg(fn:string-to-codepoints("A String"))