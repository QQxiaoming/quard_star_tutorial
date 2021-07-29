(: Name: fn-local-name-9:)
(: Description: Evaluation of the fn:local-name function argument set to a computed element node with no prefix.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(element anElement {"Some content"}))
