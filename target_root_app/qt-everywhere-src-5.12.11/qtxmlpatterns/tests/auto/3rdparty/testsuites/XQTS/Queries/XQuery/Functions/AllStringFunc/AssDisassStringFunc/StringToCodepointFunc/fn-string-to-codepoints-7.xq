(: Name: fn-string-to-codepoints-7 :)
(: Description: Evaluation of an "fn:string-to-codepoints" with argument that uses "fn:upper-case" function . :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-to-codepoints(fn:upper-case("A String"))