(: Name: fn-string-to-codepoints-8 :)
(: Description: Evaluation of an "fn:string-to-codepoints" with argument that uses "fn:lower-case" function . :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-to-codepoints(fn:lower-case("A String"))