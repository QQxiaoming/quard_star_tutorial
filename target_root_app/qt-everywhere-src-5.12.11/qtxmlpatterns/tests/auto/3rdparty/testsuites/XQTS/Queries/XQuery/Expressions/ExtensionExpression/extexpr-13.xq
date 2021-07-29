(: Name: extexpr-13 :)
(: Description: An extension expression for which the pragma is ignored and default expression is an "or" true expression ("and" operator):)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
     {fn:string(fn:true() and fn:true())}
