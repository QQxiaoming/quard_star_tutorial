(: Name: fn-codepoints-to-string-8 :)
(: Description: Evaluation of an "fn:codepoints-to-string" used as argument to "xs:string()" function and uses codepoints 65, 32, 83 116, 114, 105, 110, 103 ("A String") . :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:string(fn:codepoints-to-string((65,32,83,116,114,105,110,103)))