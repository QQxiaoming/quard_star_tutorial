(: Name: fn-local-name-11:)
(: Description: Evaluation of the fn:local-name function argument set to a directly constructed element node with a prefix.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(<p1:anElement xmlns:p1="http://example.com">Some content</p1:anElement>))
