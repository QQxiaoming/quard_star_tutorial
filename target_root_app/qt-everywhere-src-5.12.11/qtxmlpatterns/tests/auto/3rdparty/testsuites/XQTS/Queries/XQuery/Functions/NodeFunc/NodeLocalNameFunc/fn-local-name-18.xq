(: Name: fn-local-name-18:)
(: Description: Evaluation of the fn:local-name function argument set to a computed element node with prefix and a declare namespace declaration.:)
(: Use fn:string.  :)

declare namespace p1 = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(element p1:anElement{"Some content"}))