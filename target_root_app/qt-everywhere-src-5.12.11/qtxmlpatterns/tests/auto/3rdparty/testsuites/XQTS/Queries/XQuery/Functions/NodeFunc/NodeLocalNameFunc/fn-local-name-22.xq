(: Name: fn-local-name-22:)
(: Description: Evaluation of the fn:local-name function argument set to a computed attribite node with no prefix used as argument to lower-case function.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:lower-case(fn:string(fn:local-name(attribute anAttribute {"Some content"})))