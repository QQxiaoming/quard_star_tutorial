(: Name: fn-local-name-12:)
(: Description: Evaluation of the fn:local-name function argument set to a computed attribute node with no prefix.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(attribute anAttribute {"Attribute Value"}))
