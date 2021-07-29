(: Name: fn-local-name-10:)
(: Description: Evaluation of the fn:local-name function argument set to a directly constructed element node with no prefix.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(<anElement>Some content</anElement>))
