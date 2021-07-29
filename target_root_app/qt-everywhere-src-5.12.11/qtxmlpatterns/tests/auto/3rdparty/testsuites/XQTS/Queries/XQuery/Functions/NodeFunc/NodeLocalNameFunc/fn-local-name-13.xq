(: Name: fn-local-name-13:)
(: Description: Evaluation of the fn:local-name function argument set to a computed attribute node with a prefix.:)
(: Use fn:string.  :)

declare namespace p1 = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(attribute p1:anAttribute {"Attribute Value"}))
