(: Name: fn-local-name-17:)
(: Description: Evaluation of the fn:local-name function argument set to a directly constructed element node with no prefix used as argument to string-length function.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string-length(fn:string(fn:local-name(<anElement>Some content</anElement>)))