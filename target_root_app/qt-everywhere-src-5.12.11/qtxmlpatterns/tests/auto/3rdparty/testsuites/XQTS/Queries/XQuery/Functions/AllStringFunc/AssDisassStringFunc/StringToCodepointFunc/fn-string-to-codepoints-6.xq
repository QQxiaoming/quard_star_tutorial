(: Name: fn-string-to-codepoints-6 :)
(: Description: Evaluation of an "fn:string-to-codepoints" with argument that uses "xs:string()" function . :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-to-codepoints(xs:string("A String"))