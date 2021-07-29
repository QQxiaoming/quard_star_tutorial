(: Name: fn-local-name-16:)
(: Description: Evaluation of the fn:local-name function argument set to a directly constructed element node with prefix and a declare namespace declaration.:)
(: Use fn:string.  :)

declare namespace p1 = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(<p1:anElement>Some content</p1:anElement>))