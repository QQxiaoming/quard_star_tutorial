(: Name: fn-codepoints-to-string-12 :)
(: Description: Evaluation of an "fn:codepoints-to-string" that uses fn:avg/xs:integer function and codepoints 65,32,83,116,114,105,110,103. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:codepoints-to-string(xs:integer(fn:avg((65,32,83,116,114,105,110,103))))