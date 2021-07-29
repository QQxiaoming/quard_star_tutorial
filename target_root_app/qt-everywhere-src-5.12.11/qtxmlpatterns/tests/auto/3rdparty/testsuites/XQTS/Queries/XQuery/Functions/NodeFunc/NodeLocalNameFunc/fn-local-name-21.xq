(: Name: fn-local-name-21:)
(: Description: Evaluation of the fn:local-name function argument set to a computed attribite node with no prefix used as argument to upper-case function.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:upper-case(fn:string(fn:local-name(attribute anAttribute {"Some content"})))