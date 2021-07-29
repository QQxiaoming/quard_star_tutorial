(: Name: fn-local-name-20:)
(: Description: Evaluation of the fn:local-name function argument set to a directly constructed element node with no prefix used as argument to lower-case function.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:lower-case(fn:string(fn:local-name(<anElement>Some content</anElement>)))